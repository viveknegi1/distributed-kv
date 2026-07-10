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
    establishConnection();
    m_thread = std::thread(&PeerConnection::retryConnection, this);    
}

bool PeerConnection::establishConnection()
{
    {
        std::lock_guard<std::mutex> lock(m_connectionMutex);
        if (m_fileDescriptor == -1) 
        {
            close(m_fileDescriptor);
            
        }
    }

    m_fileDescriptor = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(m_fileDescriptor == - 1)
    {

        Logger::getInstance().log(Logger::Level::ERROR, "Invalid Socket Address");
        return false;

    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_portAddress);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); 
  
    // 3. Attempt the network handshake
    if (connect(m_fileDescriptor, (struct sockaddr*)&addr, sizeof(addr)) != 0) 
    {
        {
            close(m_fileDescriptor);
            std::lock_guard<std::mutex> lock(m_connectionMutex); 
            m_isConnected = false;
            Logger::getInstance().log(Logger::Level::ERROR, "Unable to connect");
             return false;
        }
    }

    std::lock_guard<std::mutex> lock(m_connectionMutex);
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
            Logger::getInstance().log(Logger::Level::ERROR, "Problem sending raw data . Setting m_isConnected to false");
            {
                std::lock_guard<std::mutex> lock(m_connectionMutex);
                close(m_fileDescriptor);
                m_fileDescriptor = -1;
                m_isConnected = false ;
            }
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
    while(!m_shouldStop)
    {
        if(!m_isConnected)
        {
            if(establishConnection())
            {
                Logger::getInstance().log(Logger::Level::INFO, "Connected to port " + std::to_string(m_portAddress));
            }
            else
            {
                Logger::getInstance().log(Logger::Level::ERROR, "Connection failed, retrying in 2 seconds");
                std::this_thread::sleep_for(std::chrono::seconds(2));
            }
        }
        else
        {
            // Already connected, check again after a short sleep
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
}