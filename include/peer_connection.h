#pragma once

#include <atomic>
#include <mutex>
#include <string>
#include <thread>

class PeerConnection
{
    private:
        std::string m_nodeID ;
        int m_portAddress;
        int m_fileDescriptor = -1;
        std::thread m_thread;
        std::atomic<bool> m_isConnected = false ;
        std::atomic<bool> m_shouldStop = false ;
        std::mutex m_connectionMutex ;

        void retryConnection();
        bool establishConnection();
        
    public:
        PeerConnection(std::string inNodeID, int inPortAddress );
        ~PeerConnection();
        bool isConnected() const ;
        void sendDataToPeers(const std::vector<uint8_t>& inRawBytesData);
};