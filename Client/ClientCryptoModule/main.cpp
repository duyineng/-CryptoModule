#include <WinSock2.h>
#include "TcpClient.h" 
#include "TcpCommunication.h"
#include "SimpleLogger.h"

#pragma comment(lib, "Ws2_32.lib")

int main()
{
    // 初始化Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) 
    {
        LOG_ERROR("init winsock fialed");
        return 1;
    }

    // 创建 TcpClient 对象
    TcpClient client;

    // 设置客户端套接字
    const std::string serverIP = "127.0.0.1";  // 替换为实际的服务器 IP
    const uint16_t serverPort = 8080;          // 替换为实际的服务器端口
    if (client.setupClientSocket(serverIP, serverPort) != 0) 
    {
        LOG_ERROR("Failed to setup client socket");
        WSACleanup();
        return 1;
    }

    // 4. 连接到服务器
    auto comm = client.connectToServer();
    if (!comm) 
    {
        LOG_ERROR("Failed to connect to server");
        WSACleanup();
        return 1;
    }

    // 使用连接进行通信
    // 例如，发送消息
    std::string message = "Hello, Server!";
    if (comm->sendMessage(message) != TcpCommunication::ErrorType::SUCCESS)
    {
        LOG_ERROR("Failed to send message");
    }
    else 
    {
        LOG_INFO("Message sent successfully");
    }

    // 接收响应
    std::string response;
    if (comm->receiveMessage(response) == TcpCommunication::ErrorType::SUCCESS)
    {
        LOG_INFO("Received response:" + response);
        LOG_ERROR("Failed to send message");
    }
    else 
    {
        LOG_ERROR("Failed to receive response");
    }

    // 清理和关闭
    WSACleanup();

    return 0;
}