#pragma once
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

// 用于TCP通信的类
class TcpCommunication
{
public:
	enum class ErrorType { SUCCESS, TIMEOUT, PEER_CLOSE, ERROR2 };

	explicit TcpCommunication(SOCKET&& socket);
	~TcpCommunication();

	ErrorType sendMessage(const std::string& data, int timeout = 100);
	ErrorType receiveMessage(std::string& outData, int timeout = 100);
	SOCKET getSocket() const;
	void closeSocket();

private:
	ErrorType sendTimeout(unsigned int timeout);
	ErrorType sendLength(const void* buffer, size_t length);
	ErrorType receiveTimeout(unsigned int timeout);
	ErrorType receiveLength(void* buffer, size_t length);

	SOCKET m_socket = INVALID_SOCKET;
};

inline SOCKET TcpCommunication::getSocket() const { return m_socket; }
