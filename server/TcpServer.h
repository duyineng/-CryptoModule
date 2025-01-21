#pragma once
#include <memory>
#include "TcpCommunication.h"

/*
 * 服务器这边不需要将m_lfd设置成非阻塞
 */
class TcpServer
{
public:
	TcpServer() = default;
	~TcpServer(); 

	int setupServerSocket(uint16_t port);
	std::unique_ptr<TcpCommunication> acceptFromClient(int timeout = 100);

private:
	int acceptTimeout(uint32_t timeout);
	inline void closeFd(); 

    int m_listenfd = -1;
};
