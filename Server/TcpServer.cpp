#include <memory>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <iostream>
#include "TcpCommunication.h"
#include "TcpServer.h"
#include "../share/SimpleLogger.h"

TcpServer::~TcpServer()
{
	if(m_listenfd != -1)
	{
		close(m_listenfd);
		m_listenfd = -1;
	}
}

int TcpServer::setupServerSocket(uint16_t port)
{
	// 创建用于监听的套接字
	m_listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (m_listenfd == -1)
	{
        LOG_ERROR("Create m_listenfd failed, error code: " + std::string(strerror(errno)));
		return -1;
	}

	// 设置端口复用
	int on = 1;
	int ret = setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	if (ret == -1)
	{
		LOG_ERROR("Failed to set port reuse, error code: " + std::string(strerror(errno)));
        closeFd();
        return -1;
	}

	// 初始化服务器地址结构
	struct sockaddr_in serverAddress;
	memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(port);
	socklen_t serverAddressLength = sizeof(serverAddress);

	// 监听文件描述符和服务器地址结构作绑定
	ret = bind(m_listenfd, (struct sockaddr*)&serverAddress, serverAddressLength);
	if (ret == -1)
	{
		LOG_ERROR("Failed to bind serverAddress with m_listenfd, error code: " + std::string(strerror(errno)));
        closeFd();
        return -1;
	}

	// 设置可同时监听的文件描述符上限
	ret = listen(m_listenfd, 128);
	if (ret == -1)
	{
		LOG_ERROR("Failed to call listen function, error code: " + std::string(strerror(errno)));
        closeFd();
        return -1;
	}

	return 0;
}

std::unique_ptr<TcpCommunication> TcpServer::acceptFromClient(int timeout)
{
	int ret = acceptTimeout(timeout);
	if(ret != 0)
	{
		LOG_ERROR("Server accept client connection failed"); 	
		return nullptr;
	}

    struct sockaddr_in clientAddress;
    socklen_t clientAddressLength = sizeof(clientAddress);
    int cfd = accept(m_listenfd, (struct sockaddr*)&clientAddress, &clientAddressLength);	// 绝对不阻塞，因为select检测到有客户端连接请求
    if (cfd == -1)
    {
        LOG_ERROR("Server call accept function failed, error code: " + std::string(strerror(errno))); 	
        return nullptr;
    }

	LOG_INFO("Accepting from client successfully"); 	
    return std::make_unique<TcpCommunication>(std::move(cfd));
    // return std::unique_ptr<TcpCommunication>(new TcpCommunication(std::move(cfd)));	// 成功，返回独占的智能指针对象
}

int TcpServer::acceptTimeout(uint32_t timeout)
{
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(m_listenfd, &readfds);

	struct timeval selectTimeout = { timeout, 0 };

	int selectRet;
	do{
		selectRet = select(m_listenfd+1, &readfds, NULL, NULL, &selectTimeout);
	} while (selectRet == -1 && errno == EINTR);

	if (selectRet < 0)
	{
		LOG_ERROR("Failed to call select function, error code: " + std::string(strerror(errno)));
        return -1;
	}
	else if (selectRet == 0)
	{
		LOG_INFO("Select function timed out");	// 当发生超时的时候，errno并不会被设置为特定的值，它仍然是之前的值
		return -1;		
	}

    return 0;
}

inline void TcpServer::closeFd()
{
	if(m_listenfd != -1)
	{
		if(close(m_listenfd) != 0)
		{
			LOG_ERROR("Failed to close m_listenfd, error code: " + std::string(strerror(errno)));
			m_listenfd = -1;
		}
		m_listenfd = -1;			
	}
}