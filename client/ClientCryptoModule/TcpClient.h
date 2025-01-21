#pragma once
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <memory>
#include "TcpCommunication.h"

#pragma comment(lib, "Ws2_32.lib")

class TcpClient
{
public:
	TcpClient() = default;
	~TcpClient();

	int setupClientSocket(const std::string& ip, uint16_t port);
	std::unique_ptr<TcpCommunication> connectToServer(int timeout = 100);
	SOCKET getSocket() const;

private:
	int connectTimeout(uint32_t timeout);
	int setBlock(SOCKET socket);
	int setNonBlock(SOCKET socket);
	void closeSocket();

protected:
	SOCKET  m_socket = INVALID_SOCKET;
	struct sockaddr_in m_serverAddress{};
	socklen_t m_serverAddressLength;
};

inline SOCKET TcpClient::getSocket() const { return m_socket; }

