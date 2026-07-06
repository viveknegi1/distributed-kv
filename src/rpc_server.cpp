#include "rpc_server.h"

#include "logger.h"
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <unistd.h> 
#include <thread>
#include "message.h"
#include "raft_node.h"

namespace
{

bool recvAll(int inSocketFd, std::vector<uint8_t>& opBuffer, size_t inNumBytes) {
    opBuffer.resize(inNumBytes);
    size_t totalRead = 0;
    while (totalRead < inNumBytes) 
    {
        ssize_t received = recv(inSocketFd, opBuffer.data() + totalRead, 
                               inNumBytes - totalRead, 0);
        if (received <= 0) 
        {
            return false;
        }
        totalRead += received;
    }
    return true;
}
}


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
    
    if(m_raftNode != nullptr)
    {
        while(true)
        {
        std::vector<uint8_t> buffer ;
        if(recvAll(socketFd , buffer, 1)) //read first byte to know what kind of data is there
        {
            if( buffer.at(0) ==  static_cast<uint8_t>(MessageType::VOTE_REQUEST) )
            {
                // Read term (4 bytes)
                std::vector<uint8_t> termBuffer;
                recvAll(socketFd, termBuffer, 4);

                // Read string length (1 byte)
                std::vector<uint8_t> lenBuffer;
                recvAll(socketFd, lenBuffer, 1);
                uint8_t strLen = lenBuffer[0];

                // Read string (strLen bytes)
                std::vector<uint8_t> strBuffer;
                recvAll(socketFd, strBuffer, strLen);

                // Read lastLogIndex (4 bytes)
                std::vector<uint8_t> indexBuffer;
                recvAll(socketFd, indexBuffer, 4);
                
                std::vector<uint8_t> resultBuffer;
                resultBuffer.reserve(termBuffer.size() + lenBuffer.size() + strBuffer.size() + indexBuffer.size());
                resultBuffer.insert(resultBuffer.end(), termBuffer.begin(), termBuffer.end());
                resultBuffer.insert(resultBuffer.end(), lenBuffer.begin(), lenBuffer.end());
                resultBuffer.insert(resultBuffer.end(), strBuffer.begin(), strBuffer.end());
                resultBuffer.insert(resultBuffer.end(), indexBuffer.begin(), indexBuffer.end());
                
                auto voteRequestStruct = MessageSerializer::deserializeVoteRequest(resultBuffer);
                auto candidateId = voteRequestStruct.candidateID;
                auto term = voteRequestStruct.term ;
                auto lastLogIndex = voteRequestStruct.lastLogIndex;
                m_raftNode->requestVote(candidateId, term , lastLogIndex);
                
            
            }

            else if(buffer.at(0) ==  static_cast<uint8_t>(MessageType::VOTE_RESPONSE))
            {

                // Read term (4 bytes)
                std::vector<uint8_t> termBuffer;
                recvAll(socketFd, termBuffer, 4);

                // Read string length (1 byte)
                std::vector<uint8_t> voteGrantedBuffer;
                recvAll(socketFd, voteGrantedBuffer, 1);

                std::vector<uint8_t> resultBuffer;
                resultBuffer.reserve(termBuffer.size() + voteGrantedBuffer.size());
                resultBuffer.insert(resultBuffer.end(), termBuffer.begin(), termBuffer.end());
                resultBuffer.insert(resultBuffer.end(), voteGrantedBuffer.begin(), voteGrantedBuffer.end());
                auto voteResponseStruct = MessageSerializer::deserializeVoteResponse(resultBuffer);
                auto term = voteResponseStruct.term ;
                auto voteGranted = voteResponseStruct.granted;
                 m_raftNode->receiveVote( term , voteGranted);
               
            }

            else if(buffer.at(0) ==  static_cast<uint8_t>(MessageType::HEARTBEAT))
            {
                 // Read term (4 bytes)
                std::vector<uint8_t> termBuffer;
                recvAll(socketFd, termBuffer, 4);

                // Read Leader ID length (1 byte)
                std::vector<uint8_t> lenBuffer;
                recvAll(socketFd, lenBuffer, 1);
                uint8_t strLen = lenBuffer[0];

                // Read Leader ID (strLen bytes)
                std::vector<uint8_t> strBuffer;
                recvAll(socketFd, strBuffer, strLen);

                
                std::vector<uint8_t> resultBuffer;
                resultBuffer.reserve(termBuffer.size() + lenBuffer.size() + strBuffer.size());
                resultBuffer.insert(resultBuffer.end(), termBuffer.begin(), termBuffer.end());
                resultBuffer.insert(resultBuffer.end(), lenBuffer.begin(), lenBuffer.end());
                resultBuffer.insert(resultBuffer.end(), strBuffer.begin(), strBuffer.end());
                
                auto heartBeatStruct = MessageSerializer::deserializeHeartbeat(resultBuffer);
                auto leaderID = heartBeatStruct.leaderID;
                auto term = heartBeatStruct.term ;
        
                m_raftNode->receiveHeartBeat(term ,leaderID);

            }
            else
            {
                Logger::getInstance().log(Logger::Level::ERROR, "Unknown message type"); 
                break ;

            }

        }

        else
        {
            Logger::getInstance().log(Logger::Level::ERROR, "Client Disconnected or insufficeint data"); 
            break ;
        }

    } 
    
    }
    close(socketFd);
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

void RpcServer::setRaftNode(RaftNode* inRaftNode)
{
    m_raftNode = inRaftNode ;
}