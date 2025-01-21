#include <iostream>
#include "TcpServer.h"
#include "TcpCommunication.h"
#include "../share/codec/RequestCodecFactory.h"
#include "../share/codec/RequestCodec.h"
#include "../share/SimpleLogger.h"

int main()
{
    // 设置服务器端的套接字
    TcpServer server;
    if(server.setupServerSocket(9527) != 0)
    {
        LOG_ERROR("Failed to setup server socket");
        return 1;
    }

    // 等待接收客户端连接请求
    std::unique_ptr<TcpCommunication> tcpComm = server.acceptFromClient();
    if(tcpComm == nullptr)
    {
        LOG_ERROR("Failed to accept from client");
        return 1;
    }

    // 等待接收客户端发来数据
    std::string str;
    TcpCommunication::ErrorType errValue = tcpComm->receiveMessage(str);
    if(errValue != TcpCommunication::ErrorType::SUCCESS)
    {
        LOG_ERROR("Failed to receive message from client");
        return 1;
    }
    std::cout << "111" << std::endl;
    // 用客户端发来数据，构建请求体工厂对象
    RequestCodecFactory reqFactory(str);
    std::unique_ptr<Codec<RequestInformation>> reqInfoPtr = reqFactory.createCodec();   // 请求体工厂对象创建请求体编解码对象
    std::optional<RequestInformation> reqInfoOpt = reqInfoPtr->decodeString();          // 请求体编解码对象将数据解码成原始的请求体
    if(reqInfoOpt == std::nullopt)
    {
        LOG_ERROR("Failed to decode string which accept from client");
        return 1;
    }

    // 输出原始请求体的数据
    RequestInformation reqInfo = reqInfoOpt.value();
    std::cout << "reqInfo.cmdType == " << reqInfo.cmdType <<std::endl;
    std::cout << "reqInfo.serverId == " << reqInfo.serverId <<std::endl;
    std::cout << "reqInfo.clientId == " << reqInfo.clientId <<std::endl;
    std::cout << "reqInfo.data == " << reqInfo.data <<std::endl;
    std::cout << "reqInfo.sign == " << reqInfo.sign <<std::endl;
}