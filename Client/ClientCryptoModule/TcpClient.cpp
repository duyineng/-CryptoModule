#include <memory>
#include <fcntl.h>
#include <iostream>
#include <cstring>
#include "TcpCommunication.h"
#include "TcpClient.h" 
#include "SimpleLogger.h" 

TcpClient::~TcpClient()
{
    closeSocket();
}

int TcpClient::setupClientSocket(const std::string& ip, uint16_t port)
{
    // 输入端口检测
    constexpr uint16_t MIN_PORT = 1;
    constexpr uint16_t MAX_PORT = 65535;
    if (port < MIN_PORT || port > MAX_PORT)
    {
        LOG_ERROR("Invalid port number");
        return -1;
    }

    // 创建客户端套接字
    m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_socket == INVALID_SOCKET)
    {
        LOG_ERROR("Create socket error, error code: " + std::to_string(WSAGetLastError()));
        return -1;
    }

    // 设置服务器的地址结构，不是绑定
    m_serverAddress.sin_family = AF_INET;
    int flag = inet_pton(AF_INET, ip.c_str(), &m_serverAddress.sin_addr.s_addr); // 将IP地址从字符串格式转换为网络字节序的二进制格式
    if (flag != 1)
    {
        LOG_ERROR( "Set server address struct error, error code: " + std::to_string(WSAGetLastError()));
        closeSocket();
        return -1;
    }
    m_serverAddress.sin_port = htons(port);
    m_serverAddressLength = sizeof(m_serverAddress);

    return 0;
}

std::unique_ptr<TcpCommunication> TcpClient::connectToServer(int timeout)
{
    // 开始连接服务器，并设置连接超时时间
    int flag = connectTimeout(timeout);
    if (flag != 0)
    {
        LOG_ERROR( "Failed to connect to server");
        return nullptr;
    }

    return std::make_unique<TcpCommunication>(std::move(m_socket));
    // return std::unique_ptr<TcpCommunication>(new TcpComm(m_socket));  了解一下区别
}

int TcpClient::connectTimeout(uint32_t timeout)
{
    if (timeout == 0)
    {
        LOG_ERROR( "Timeout value is invalid");
        return -1;
    }

    // 将文件描述符设置成非阻塞，用于connect连接
    if (setNonBlock(m_socket) == -1)
    {
        LOG_ERROR( "Failed to set non-blocking mode during connection attempt");
        return -1;  
    }

    // 非阻塞模式连接
    int ret = connect(m_socket, reinterpret_cast<struct sockaddr*>(&m_serverAddress), m_serverAddressLength);
    if (ret == 0)
    {
        // 立即连接成功，这种情况很少见
        LOG_INFO("Connection established immediately");
        return setBlock(m_socket);
    }
    else  
    {
        // 此时ret == SOCKET_ERROR
        if (WSAGetLastError() != WSAEWOULDBLOCK)    
        {   
            // 连接立即失败，但不是因为正在进行中
            LOG_ERROR( "Connection failed immediately, but not because it was in progess, error code: " + std::to_string(WSAGetLastError()));
            setBlock(m_socket);
            return -1;
        }
        else    
        {
            // 连接立即失败，但表示此时连接正在进行中，此时WSAGetLastError() == WSAEWOULDBLOCK
            fd_set writefds;
            FD_ZERO(&writefds);
            FD_SET(m_socket, &writefds);

            struct timeval timeval = { timeout, 0 };

            /*
             * 1. m_socket 已被设置为非阻塞模式。
             * 2. connect() 调用后立即返回，此时连接可能尚未建立。
             * 3. 随后的 select() 调用用于监视这个特殊状态的套接字：
             *    - 即使 m_socket 套接字本身可写，select() 也不会立即返回。
             *    - select() 会等待直到以下情况之一发生：
             *      a) 连接成功建立
             *      b) 连接尝试失败
             *      c) 达到指定的超时时间
             * 4. 这种机制允许我们在非阻塞模式下实现带超时的连接尝试。
             */
            do {
                ret = select(0, nullptr, &writefds, nullptr, &timeval);    // 在windows中，第一个参数是被忽略的，只需要writefds就可以
            } while (ret == -1 && WSAGetLastError() == EINTR);      // 被信号打断

            if (ret == 0)
            {
                // 连接超时
                LOG_ERROR("Select() timed out");    // WSAGetLastError()不会有错误码
                setBlock(m_socket);
                return -1;
            }
            else if (ret == SOCKET_ERROR)
            {
                // select 错误
                LOG_ERROR("Select() error, error code: " + std::to_string(WSAGetLastError()));
                setBlock(m_socket);
                return -1;
            }
            else
            {
                // select 返回 1，检查连接是否真的成功
                int error;
                socklen_t errorLength = sizeof(error);
                if (getsockopt(m_socket, SOL_SOCKET, SO_ERROR, (char*)&error, &errorLength) == SOCKET_ERROR)
                {
                    LOG_ERROR("Getsockopt() error, errno code:" + std::to_string(WSAGetLastError()));
                    setBlock(m_socket);
                    return -1;
                }

                if (error != 0)
                {
                    // 连接失败
                    LOG_ERROR("getsockopt()'s error != 0, error value: " + std::to_string(error));
                    setBlock(m_socket);
                    return -1;
                }
            }      
        }
    }

    // 连接成功，设置回阻塞模式
    LOG_INFO("Connect to server successedd");
    return setBlock(m_socket);
}
 
// 将文件描述符设置成非阻塞
int TcpClient::setNonBlock(SOCKET socket)
{
    u_long mode = 1;
    if (ioctlsocket(socket, FIONBIO, &mode) == SOCKET_ERROR)
    {
        LOG_ERROR( "Failed to set the file descriptor to non-blocking mode, error code: " + std::to_string(WSAGetLastError()));
        return -1;
    }
    return 0;
}

int TcpClient::setBlock(SOCKET socket)
{
    u_long mode = 0;
    if (ioctlsocket(socket, FIONBIO, &mode) == SOCKET_ERROR)
    {
        LOG_ERROR( "Failed to set the file descriptor to blocking mode, error code: " + std::to_string(WSAGetLastError()));
        return -1;
    }
    return 0;
}

void TcpClient::closeSocket()
{
    if (m_socket != INVALID_SOCKET)
    {
        if (closesocket(m_socket) == SOCKET_ERROR)
        {
            LOG_ERROR( "Close socket error, error code: " + std::to_string(WSAGetLastError()));
        }
        m_socket = INVALID_SOCKET;
    }
}

