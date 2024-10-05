#pragma once
#include <string>

#pragma comment(lib, "Ws2_32.lib")

// 用于TCP通信的类
class TcpCommunication
{
public:
	enum class ErrorType { SUCCESS, TIMEOUT, PEER_CLOSE, ERROR2 };

	explicit TcpCommunication(int&& fd);
	~TcpCommunication();

	ErrorType sendMessage(const std::string& data, int timeout = 100);
	ErrorType receiveMessage(std::string& outData, int timeout = 100);
	int getFd() const;
	void closeFd();

private:
	ErrorType sendTimeout(unsigned int timeout);
	ErrorType sendLength(const void* buffer, size_t length);
	ErrorType receiveTimeout(unsigned int timeout);
	ErrorType receiveLength(void* buffer, size_t length);

	int m_fd = -1;
};

inline int TcpCommunication::getFd() const { return m_fd; }
