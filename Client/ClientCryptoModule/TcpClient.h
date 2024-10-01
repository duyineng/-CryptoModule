#pragma once
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <memory>
#include "TcpComm.h"

#pragma comment(lib, "Ws2_32.lib")

class TcpClient
{
public:
	TcpClient() = default;
	~TcpClient();
	int setupClientSocket(const std::string& ip, uint16_t port);
	std::unique_ptr<TcpComm> connToHost(int timeout = 100);
	SOCKET getFd() const;

private:
	int connectTimeout(int timeout);
	int setBlock(SOCKET fd);
	int setNonBlock(SOCKET fd);
	void closeFd();

protected:
	SOCKET  m_fd = INVALID_SOCKET;
	struct sockaddr_in m_servAddr {};	
	socklen_t m_servLen;
};

inline SOCKET TcpClient::getFd() const { return m_fd; }

