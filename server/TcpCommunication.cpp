#include <vector>
#include <iostream>
#include <fcntl.h>
#include <cstring>
#include <vector>
#include <arpa/inet.h>
#include "TcpCommunication.h"
#include "../share/SimpleLogger.h" 
#include <unistd.h>

TcpCommunication::TcpCommunication(int&& fd)
	: m_fd(fd)
{
	fd = -1;
}

TcpCommunication::~TcpCommunication()
{
	closeFd();
}

TcpCommunication::ErrorType TcpCommunication::sendMessage(const std::string& data, int timeout)
{
	// 发送数据前，检查套接字写缓冲区是否可写，并设置检查超时时间
	ErrorType ret = sendTimeout(timeout);
	if (ret != ErrorType::SUCCESS)
	{
		LOG_ERROR("The write buffer is currently unavailabel for writing");
		return ret;
	}
	
	// 数据准备
	uint32_t dataSize = static_cast<uint32_t>(data.length());
	uint32_t dataSizeNetByte = htonl(dataSize);				// 转换为网络字节序
	uint32_t totalSize = dataSize + sizeof(uint32_t);	// 防止TCP粘包，多加4字节，用于指示数据长度
	std::vector<unsigned char> sendBuffer(totalSize);
	
	std::memcpy(sendBuffer.data(), &dataSizeNetByte, sizeof(uint32_t));
	std::memcpy(sendBuffer.data() + sizeof(uint32_t), data.data(), dataSize);

	// 数据发送
	ret = sendLength(sendBuffer.data(), sendBuffer.size());
	if (ret != ErrorType::SUCCESS)
	{
		LOG_ERROR("Send message failed");
		return ErrorType::ERROR2;
	}

	LOG_INFO("Send message successfully");
	return ErrorType::SUCCESS;
}

// 接收数据
TcpCommunication::ErrorType TcpCommunication::receiveMessage(std::string& outData, int timeout)
{
	std::cout << "333" << std::endl;
	// 检测套接字读缓冲区是否可读，并设置检测超时时间
	ErrorType ret = receiveTimeout(timeout);
	if (ret != ErrorType::SUCCESS)
	{
		LOG_ERROR("recvTimeout() is not successfully");
		return ErrorType::ERROR2;
	}
	std::cout << "222" << std::endl;
	uint32_t dataLengthNetByte = 0;
	ret = receiveLength(&dataLengthNetByte, 4);	// 绝对不阻塞，因为已经检测到套接字可读。先读取前4个字节，并写入dataLengthNetByte中
	if (ret != ErrorType::SUCCESS)
	{
		LOG_ERROR("recvn() dataSizeNetByte is not successfully");
		return ErrorType::ERROR2;
	}

	uint32_t dataLength = ntohl(dataLengthNetByte);	 // 把网络字节序转换成本地字节序
	std::vector<char> recvBuf(dataLength);

	ret = receiveLength(recvBuf.data(), recvBuf.size());	// 读到recvBuf.data()里面，读取recvBuf.size()个字节
	if (ret != ErrorType::SUCCESS)
	{
		LOG_ERROR("recvn() data is not successfully");
		return ErrorType::ERROR2;
	}

	outData.assign(recvBuf.data(), recvBuf.size());
	LOG_INFO("recv message successfully");
	return ErrorType::SUCCESS;
}

// 关闭套接字
void TcpCommunication::closeFd()
{
	if (m_fd != -1)
	{
		if (close(m_fd) == -1)
		{
			LOG_ERROR("Close fd error, error code: " + std::string(std::strerror(errno)));
		}
		m_fd = -1;
	}
}

// 检查套接字写缓冲区是否可写，并设置检查超时时间
// 一般本端套接字都可写，但是如果本端写缓冲区满了，就会阻塞。或者对端读缓冲区满了，也会通知本端阻塞
TcpCommunication::ErrorType TcpCommunication::sendTimeout(unsigned int timeout)
{
	// 设置select函数所监听的写缓冲区集合
	fd_set writefds;
	FD_ZERO(&writefds);
	FD_SET(m_fd, &writefds);

	// 设置select函数的监听超时时长
	timeval selectTimeout = { timeout,0 };

	int ret;
	do {
		ret = select(m_fd + 1, NULL, &writefds, NULL, &selectTimeout);
	} while (ret == -1 && errno == EINTR);
	if (ret == -1)
	{
		LOG_ERROR("select() error, error code: " + std::string(std::strerror(errno)));
		return ErrorType::ERROR2;	
	}
	else if (ret == 0)
	{
		LOG_WARNING("select() timeout");
		return ErrorType::TIMEOUT;
	}

	LOG_INFO("File descriptor is writable");
	return ErrorType::SUCCESS;	
}

// buffer指向要传输的数据的缓冲区，length为数据的大小
TcpCommunication::ErrorType TcpCommunication::sendLength(const void* buffer, size_t length)
{
	size_t remainLength = length;
	const char* currentBuffer = static_cast<const char*>(buffer);

	while (remainLength > 0)
	{
		int nSend = send(m_fd, currentBuffer, remainLength, 0);
		if (nSend == -1)
		{
			if (errno == EINTR)
			{
				continue;
			}
			else
			{
				LOG_ERROR("Failed to call send function, error code: " + std::string(strerror(errno)));
				return ErrorType::ERROR2;
			}
		}
		else if (nSend == 0)	// 表示当前写缓冲区已满，再来一次
		{
			continue;
		}
		else if (nSend > 0)
		{
			remainLength -= nSend;
			currentBuffer += nSend;
		}
	}

	return ErrorType::SUCCESS;
}

TcpCommunication::ErrorType TcpCommunication::receiveTimeout(unsigned int timeout)
{
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(m_fd, &readfds);

	timeval selectTimeout = { timeout,0 };

	int ret;
	do {
		ret = select(m_fd + 1, &readfds, NULL, NULL, &selectTimeout);
	} while (ret == -1 && errno == EINTR);	// 被信号打断

	if (ret == -1)
	{
		LOG_ERROR("select error, error code: " + std::string(strerror(errno)));
		return ErrorType::ERROR2;
	}
	else if (ret == 0)
	{
		LOG_WARNING("select timeout");
		return ErrorType::TIMEOUT;
	}

	return ErrorType::SUCCESS;
}

// 读取到buffer缓冲区，length为要读的字节数
TcpCommunication::ErrorType TcpCommunication::receiveLength(void* buffer, size_t length)
{
	size_t remainLength = length;
	char* currentBuffer = static_cast<char*>(buffer);

	while (remainLength > 0)
	{
		int nRecv = recv(m_fd, currentBuffer, remainLength, 0);
		if (nRecv == -1)
		{
			if (errno == EINTR)
			{
				continue;
			}
			else
			{
				LOG_ERROR("recv() error, error code: " + std::string(strerror(errno)));
				return ErrorType::ERROR2;
			}
		}
		else if (nRecv == 0)
		{
			LOG_WARNING("Peer closed connection");
			return ErrorType::PEER_CLOSE;
		}
		else if (nRecv > 0)
		{
			remainLength -= nRecv;
			currentBuffer += nRecv;
		}
	}

	return ErrorType::SUCCESS;
}
