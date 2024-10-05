#include "TcpServer.h"
#include "TcpCommunication.h"

int main()
{
    TcpServer server;
    server.setupServerSocket(9527);
    std::unique_ptr<TcpCommunication> tcpComm = server.acceptFromClient();
    if(tcpComm == nullptr)
    {
        return 1;
    }
}