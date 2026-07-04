#include "peer_connection.h"
#include "logger.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <unistd.h>
#include <thread>
#include <chrono>


 PeerConnection::PeerConnection(std::string inNodeID, int inPortAddress) : m_nodeID(inNodeID) , m_portAddress(inPortAddress)
 { 
    // The constructor immediately invokes the connection logic
    bool success = establishConnection();
    if (!success) 
    { 
        Logger::getInstance().log(Logger::Level::ERROR, "Initialization warning: Failed to connect to 127.0.0.1 on port" + std::to_string(m_portAddress));
        m_thread = std::thread(&PeerConnection::retryConnection, this);    
    }

 }

bool PeerConnection::establishConnection()
{
    m_fileDescriptor = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_fileDescriptor == -1) {
        Logger::getInstance().log(Logger::Level::ERROR, "Invalid Socket Address");
        return false;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_portAddress);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); 

    // 3. Attempt the network handshake
    if (connect(m_fileDescriptor, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        close(m_fileDescriptor);
        m_fileDescriptor = -1;
        m_isConnected = false;
        Logger::getInstance().log(Logger::Level::ERROR, "Unable to connect");
        return false;
    }

    m_isConnected = true; 
    return true;

}


bool PeerConnection::isConnected() const
{
  return m_isConnected ;
}
        
void PeerConnection::sendDataToPeers(const std::vector<uint8_t>& inRawBytesData)
{
    size_t totalSent = 0;
    while (totalSent < inRawBytesData.size()) 
    {
        ssize_t sent = send(m_fileDescriptor, 
                        inRawBytesData.data() + totalSent, 
                        inRawBytesData.size() - totalSent, 0);
        if (sent == -1) 
        {
            Logger::getInstance().log(Logger::Level::ERROR, "Problem sending raw data");
            break;
        }
        totalSent += sent;
    }   
}

 PeerConnection::~PeerConnection()
 {
    m_shouldStop = true;
    if(m_fileDescriptor != -1)
    {
        close(m_fileDescriptor);
        m_fileDescriptor = -1;
    }
    if(m_thread.joinable())
    { 
        m_thread.join();
    }
 }

 void PeerConnection::retryConnection()
 {
    if (m_isConnected) 
    {
       return ; // already connected ;
    }

    while(!m_shouldStop) 
    {
        if (establishConnection()) 
        {
            Logger::getInstance().log(Logger::Level::INFO, "Connected to successfully to port" + std::to_string(m_portAddress));
            m_isConnected = true ;
            break; 
        }

        // If it fails, log it, sleep for 2 seconds, and try again
        Logger::getInstance().log(Logger::Level::ERROR, "Connection failed will try in 2 seconds");
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

 }