#pragma once
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

// 用于TCP通信的类
class TcpComm
{
public:
	enum class ErrorType { SUCCESS, TIMEOUT, PEER_CLOSE, ERROR2 };


	explicit TcpComm(SOCKET&& fd);
	~TcpComm();

	ErrorType sendMsg(const std::string& data, int timeout = 100);
	ErrorType recvMsg(std::string& outData, int timeout = 100);
	SOCKET getFd() const;
	void closeFd();

private:
	ErrorType sendTimeout(unsigned int timeout);
	ErrorType sendn(const void* buf, size_t bufLen);
	ErrorType recvTimeout(unsigned int timeout);
	ErrorType recvn(void* buf, int count);

	SOCKET m_fd = INVALID_SOCKET;
};

inline SOCKET TcpComm::getFd() const { return m_fd; }
