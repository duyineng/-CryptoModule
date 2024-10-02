#include <memory>
#include <fcntl.h>
#include <iostream>
#include <cstring>
#include "TcpComm.h"
#include "TcpClient.h" 
#include "SimpleLogger.h" 

TcpClient::~TcpClient()
{
    closeFd();
}

int TcpClient::setupClientSocket(const std::string& ip, uint16_t port)
{
    // 输入端口检测
    constexpr uint16_t MIN_PORT = 1;
    constexpr uint16_t MAX_PORT = 65535;
    if (port < MIN_PORT || port > MAX_PORT)
    {
        LOG_ERROR("invalid port number");
        return -1;
    }

    // 创建客户端套接字
    m_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_fd == INVALID_SOCKET)
    {
        LOG_ERROR("create socket error, error code: " + std::to_string(WSAGetLastError()));
        return -1;
    }

    // 设置服务器的地址结构，不是绑定
    m_servAddr.sin_family = AF_INET;
    int flag = inet_pton(AF_INET, ip.c_str(), &m_servAddr.sin_addr.s_addr); // 将IP地址从字符串格式转换为网络字节序的二进制格式
    if (flag != 1)
    {
        LOG_ERROR( "set server address struct error, error code: " + std::to_string(WSAGetLastError()));
        closeFd();
        return -1;
    }
    m_servAddr.sin_port = htons(port);

    m_servLen = sizeof(m_servAddr);

    return 0;
}

std::unique_ptr<TcpComm> TcpClient::connToHost(int timeout)
{
    // 开始连接服务器，并设置连接超时时间
    int flag = connectTimeout(timeout);
    if (flag < 0)
    {
        if (WSAGetLastError() == WSAETIMEDOUT)  // 连接超时
        {
            LOG_ERROR( "connect to server timed out, error code: " + std::to_string(WSAGetLastError()));
        }
        else                                    
        {
            LOG_ERROR( "connect to server error, error code: " + std::to_string(WSAGetLastError()));
        }
        return nullptr;
    }

    return std::make_unique<TcpComm>(std::move(m_fd));
    // return std::unique_ptr<TcpComm>(new TcpComm(m_fd));  了解一下区别
}

int TcpClient::connectTimeout(unsigned int timeout)
{
    if (timeout == 0)
    {
        LOG_ERROR( "timeout value is invalid");
        return -1;
    }

    // 将文件描述符设置成非阻塞，用于connect连接
    if (setNonBlock(m_fd) == -1)
    {
        LOG_ERROR( "failed to set non-blocking mode during connection attempt");
        return -1;  
    }

    // 非阻塞模式连接
    int ret = connect(m_fd, reinterpret_cast<struct sockaddr*>(&m_servAddr), m_servLen);
    if (ret == 0)
    {
        // 立即连接成功，这种情况很少见
        LOG_INFO("connect established immediately");
        return setBlock(m_fd);
    }
    else  
    {
        // 此时ret == SOCKET_ERROR
        if (WSAGetLastError() != WSAEWOULDBLOCK)    
        {   
            // 连接立即失败，但不是因为正在进行中
            LOG_ERROR( "connection failed immediately, but not because it was in progess, error code: " + std::to_string(WSAGetLastError()));
            setBlock(m_fd);
            return -1;
        }
        else    
        {
            // 此时WSAGetLastError() == WSAEWOULDBLOCK，连接立即失败，但表示此时连接正在进行中
            fd_set wFdSet;
            FD_ZERO(&wFdSet);
            FD_SET(m_fd, &wFdSet);

            struct timeval tv = { timeout, 0 };

            /*
             * 1. m_fd 已被设置为非阻塞模式。
             * 2. connect() 调用后立即返回，此时连接可能尚未建立。
             * 3. 随后的 select() 调用用于监视这个特殊状态的套接字：
             *    - 即使 m_fd 套接字本身可写，select() 也不会立即返回。
             *    - select() 会等待直到以下情况之一发生：
             *      a) 连接成功建立
             *      b) 连接尝试失败
             *      c) 达到指定的超时时间
             * 4. 这种机制允许我们在非阻塞模式下实现带超时的连接尝试。
             */
            do {
                ret = select(0, nullptr, &wFdSet, nullptr, &tv);    // 在windows中，第一个参数是被忽略的，只需要wFdSet就可以
            } while (ret == -1 && WSAGetLastError() == EINTR);      // 被信号打断

            if (ret == 0)
            {
                // 连接超时
                LOG_ERROR("select() timed out, error code: " + std::to_string(WSAGetLastError()));
                setBlock(m_fd);
                return -1;
            }
            else if (ret == SOCKET_ERROR)
            {
                // select 错误
                LOG_ERROR("select() error, error code: " + std::to_string(WSAGetLastError()));
                setBlock(m_fd);
                return -1;
            }
            else
            {
                // select 返回 1，检查连接是否真的成功
                int error;
                socklen_t errorLen = sizeof(error);
                if (getsockopt(m_fd, SOL_SOCKET, SO_ERROR, (char*)&error, &errorLen) == SOCKET_ERROR)
                {
                    LOG_ERROR("getsockopt() error: " + std::to_string(WSAGetLastError()));
                    setBlock(m_fd);
                    return -1;
                }

                if (error != 0)
                {
                    // 连接失败
                    LOG_ERROR("getsockopt()'s error != 0, error code: " + std::to_string(WSAGetLastError()));
                    setBlock(m_fd);
                    return -1;
                }
            }      
        }
    }

    // 连接成功，设置回阻塞模式
    LOG_INFO("connect to server successedd");
    return setBlock(m_fd);
}
 
// 将文件描述符设置成非阻塞
int TcpClient::setNonBlock(SOCKET fd)
{
    u_long mode = 1;
    if (ioctlsocket(fd, FIONBIO, &mode) == SOCKET_ERROR)
    {
        LOG_ERROR( "failed to set the file descriptor to non-blocking mode, error code: " + std::to_string(WSAGetLastError()));
        return -1;
    }
    return 0;
}

int TcpClient::setBlock(SOCKET fd)
{
    u_long mode = 0;
    if (ioctlsocket(fd, FIONBIO, &mode) == SOCKET_ERROR)
    {
        LOG_ERROR( "failed to set the file descriptor to blocking mode, error code: " + std::to_string(WSAGetLastError()));
        return -1;
    }
    return 0;
}

void TcpClient::closeFd()
{
    if (m_fd != INVALID_SOCKET)
    {
        if (closesocket(m_fd) == SOCKET_ERROR)
        {
            LOG_ERROR( "close socket error, error code: " + std::to_string(WSAGetLastError()));
        }
        m_fd = INVALID_SOCKET;
    }
}

