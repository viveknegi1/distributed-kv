#include "rpc_server.h"

#include "logger.h"
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <unistd.h> 
#include <thread>


RpcServer::RpcServer(int portNum)
{
    m_portNumber = portNum ;
    start();
}

void RpcServer::start()
{
    if(initialize())
    {
        if(bindSocket())
        {
            if (listen(m_serverSocket, 10) == -1)
            {
                Logger::getInstance().log(Logger::Level::ERROR, "Failed to listen on socket");
                return;
            }
            m_listenerThread = std::thread(&RpcServer::startListening, this);
        }
    }
}


bool RpcServer::initialize()
{
    m_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_serverSocket == -1) 
    {
        Logger::getInstance().log(Logger::Level::ERROR, "Failed to create socket");
        return false;
    }
    int opt = 1;
    setsockopt(m_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    return true ;

}

bool RpcServer::bindSocket()
{
    sockaddr_in in_sockaddrStruct ; 
    in_sockaddrStruct.sin_family = AF_INET ;
    in_sockaddrStruct.sin_port = htons(m_portNumber) ; //converts byte order 
    in_sockaddrStruct.sin_addr.s_addr = INADDR_ANY ;

    int result = bind(m_serverSocket , (sockaddr*)&in_sockaddrStruct, sizeof(in_sockaddrStruct));
    if( result == -1)
    {
        Logger::getInstance().log(Logger::Level::ERROR, "Failed to bind the socket");
        return false;
    }

    return true ;
}

void RpcServer::startListening()
{
     while(!m_shouldStop)
    {           
        int socketFd = acceptConnection(); 
        if(socketFd >= 0 )
        {
            m_peerConnections.push_back( std::thread (&RpcServer::handleConnection, this, socketFd) );
        } 
    }
}        

int RpcServer::acceptConnection()
{
    sockaddr_in clientAddr;
    socklen_t clientSize = sizeof(clientAddr);
    int clientFd = accept(m_serverSocket, (sockaddr*)&clientAddr, &clientSize);

    if(clientFd == -1)
    {
        Logger::getInstance().log(Logger::Level::ERROR, "Failed to accept the client");      
    }
    return clientFd;
}
        
void RpcServer::handleConnection(int socketFd)
{
    Logger::getInstance().log(Logger::Level::INFO, "Connection request was recieved for file descriptor " + std::to_string(socketFd));
}

RpcServer::~RpcServer()
{
    m_shouldStop = true;
  
    for( auto& thread: m_peerConnections)
    {
        thread.join();
    }
    m_listenerThread.join();
    if(m_serverSocket != -1)
    {
        close(m_serverSocket) ;
        m_serverSocket = -1;
    }
}