#pragma once
#include <string>
#include <thread>
#include <atomic>
#include <mutex>

class PeerConnection
{

    private:
        std::string m_nodeID ;
        int m_portAddress;
        int m_fileDescriptor = -1;
        void retryConnection();
        std::thread m_thread;
        std::atomic<bool> m_isConnected = false ;
        std::atomic<bool> m_shouldStop = false ;
        bool establishConnection();
        std::mutex m_connectionMutex ;
    
    
    public:
        PeerConnection(std::string inNodeID, int inPortAddress );
        ~PeerConnection();
        bool isConnected() const ;
        void sendDataToPeers(const std::vector<uint8_t>& inRawBytesData);

};