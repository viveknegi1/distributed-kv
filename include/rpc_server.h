#pragma once
#include <vector>
#include <thread>
#include <atomic>
class RaftNode;

class RpcServer
{

    public:
        RpcServer(int portNum);
        void start();
        ~RpcServer();
        void setRaftNode(RaftNode* inRaftNode);
        
    private: 
        std::vector<std::thread> m_peerConnections ;
        std::thread m_listenerThread;
        std::atomic<bool> m_shouldStop = false ;
        int m_serverSocket = -1 ;
        int m_portNumber ;
        RaftNode* m_raftNode  = nullptr ; 
        bool initialize();
        bool bindSocket();
        void startListening();
        int acceptConnection();
        void handleConnection(int socketFd);

};