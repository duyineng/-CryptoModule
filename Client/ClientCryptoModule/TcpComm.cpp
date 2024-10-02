#include <vector>
#include <iostream>
#include <fcntl.h>
#include <cstring>
#include <vector>
#include <iostream>
#include "TcpComm.h"
#include "SimpleLogger.h" 

TcpComm::TcpComm(SOCKET&& fd)
	: m_fd(fd)
{
	fd = INVALID_SOCKET;
}

TcpComm::~TcpComm()
{
	closeFd();
}

TcpComm::ErrorType TcpComm::sendMsg(const std::string& data, int timeout)
{
	// 发送数据前，检查套接字写缓冲区是否可写，并设置检查超时时间
	ErrorType ret = sendTimeout(timeout);
	if (ret != ErrorType::SUCCESS)
	{
		LOG_ERROR("the write buffer is currently unavailabel for writing");
		return ret;
	}
	
	// 数据准备
	uint32_t dataSize = static_cast<uint32_t>(data.size());
	uint32_t dataSizeNetByte = htonl(dataSize);				// 转换为网络字节序
	uint32_t totalSize = dataSize + sizeof(uint32_t);	// 防止TCP粘包，多加4字节，用于指示数据长度
	std::vector<unsigned char> sendBuffer(totalSize);
	
	std::memcpy(sendBuffer.data(), &dataSizeNetByte, sizeof(uint32_t));
	std::memcpy(sendBuffer.data() + sizeof(uint32_t), data.data(), dataSize);

	// 数据发送
	ret = sendn(sendBuffer.data(), sendBuffer.size());
	if (ret != ErrorType::SUCCESS)
	{
		LOG_ERROR("Send message failed");
		return ErrorType::ERROR2;
	}

	LOG_INFO("Send message successfully");
	return ErrorType::SUCCESS;
}

// 接收数据
TcpComm::ErrorType TcpComm::recvMsg(std::string& outData, int timeout)
{
	// 检测套接字读缓冲区是否可读，并设置检测超时时间
	ErrorType ret = recvTimeout(timeout);
	if (ret != ErrorType::SUCCESS)
	{
		LOG_ERROR("recvTimeout() is not successfully");
		return ErrorType::ERROR2;
	}

	uint32_t dataSizeNetByte = 0;
	ret = recvn(&dataSizeNetByte, 4);	// 绝对不阻塞，因为已经检测到套接字可读。先读取前4个字节，并写入dataSizeNetByte中
	if (ret != ErrorType::SUCCESS)
	{
		LOG_ERROR("recvn() dataSizeNetByte is not successfully");
		return ErrorType::ERROR2;
	}

	uint32_t dataSize = ntohl(dataSizeNetByte);	 // 把网络字节序转换成本地字节序
	std::vector<char> recvBuf(dataSize);

	ret = recvn(recvBuf.data(), recvBuf.size());	// 读到recvBuf.data()里面，读取recvBuf.size()个字节
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
void TcpComm::closeFd()
{
	if (m_fd != INVALID_SOCKET)
	{
		if (closesocket(m_fd) == SOCKET_ERROR)
		{
			LOG_ERROR("close socket error, error code: " + std::to_string(WSAGetLastError()));
		}
		m_fd = INVALID_SOCKET;
	}
}

// 检查套接字写缓冲区是否可写，并设置检查超时时间
// 一般本端套接字都可写，但是如果本端写缓冲区满了，就会阻塞。或者对端读缓冲区满了，也会通知本端阻塞
TcpComm::ErrorType TcpComm::sendTimeout(unsigned int timeout)
{
	// 设置select函数所监听的写缓冲区集合
	fd_set wFdset;
	FD_ZERO(&wFdset);
	FD_SET(m_fd, &wFdset);

	// 设置select函数的监听超时时长
	timeval selectTimeout = { timeout,0 };

	int ret;
	do {
		ret = select(0, NULL, &wFdset, NULL, &selectTimeout);	
	} while (ret == SOCKET_ERROR && WSAGetLastError() == WSAEINTR);
	if (ret == SOCKET_ERROR)
	{
		LOG_ERROR("select() error, error code: " + std::to_string(WSAGetLastError()));
		return ErrorType::ERROR2;	
	}
	else if (ret == 0)
	{
		LOG_WARNING("select() timeout");
		return ErrorType::TIMEOUT;
	}

	LOG_INFO("socket is writable");
	return ErrorType::SUCCESS;	
}

// buf为要传输的数据，len为数据的大小
TcpComm::ErrorType TcpComm::sendn(const void* buf, size_t len)
{
	size_t leftLen = len;	
	const char* tempBuf = static_cast<const char*>(buf);

	while (leftLen > 0)
	{
		int nWritten = send(m_fd, tempBuf, leftLen, 0);
		if (nWritten == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAEINTR)
			{
				continue;
			}
			else
			{
				LOG_ERROR("send error, error code: " + std::to_string(WSAGetLastError()));
				return ErrorType::ERROR2;
			}
		}
		else if (nWritten == 0)	// 表示当前写缓冲区已满，再来一次
		{
			continue;
		}
		else if (nWritten > 0)
		{
			leftLen -= nWritten;
			tempBuf += nWritten;
		}
	}

	return ErrorType::SUCCESS;
}

TcpComm::ErrorType TcpComm::recvTimeout(unsigned int timeout)
{
	fd_set rFdset;
	FD_ZERO(&rFdset);
	FD_SET(m_fd, &rFdset);

	timeval selectTimeout = { timeout,0 };

	int ret;
	do {
		ret = select(0, &rFdset, NULL, NULL, &selectTimeout);
	} while (ret == SOCKET_ERROR && WSAGetLastError() == WSAEINTR);	// 被信号打断

	if (ret == SOCKET_ERROR)
	{
		LOG_ERROR("select error, error code: " + std::to_string(WSAGetLastError()));
		return ErrorType::ERROR2;
	}
	else if (ret == 0)
	{
		LOG_WARNING("select timeout");
		return ErrorType::TIMEOUT;
	}

	return ErrorType::SUCCESS;
}

// 读取到buf缓冲区，len为要读的字节数
TcpComm::ErrorType TcpComm::recvn(void* buf, int len)
{
	size_t leftLen = len;
	char* tempBuf = static_cast<char*>(buf);

	while (leftLen > 0)
	{
		int nRead = recv(m_fd, tempBuf, leftLen, 0);
		if (nRead == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAEINTR)
			{
				continue;
			}
			else
			{
				LOG_ERROR("recv() error, error code: " + std::to_string(WSAGetLastError()));
				return ErrorType::ERROR2;
			}
		}
		else if (nRead == 0)
		{
			LOG_WARNING("peer closed connection");
			return ErrorType::PEER_CLOSE;
		}
		else if (nRead > 0)
		{
			leftLen -= nRead;
			tempBuf += nRead;
		}
	}

	return ErrorType::SUCCESS;
}
