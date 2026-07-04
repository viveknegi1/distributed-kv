#include "peer_manager.h"
#include "logger.h"

PeerManager::PeerManager(const NodeConfig& nodeConfig)
{
    auto allNodesData = nodeConfig.getAllOtherNodes();
    for(const auto& nodeData: allNodesData)
    {
        auto nodeID = nodeData.nodeID;
        auto portAddress = nodeData.portAddress;
        auto peerConnectionPointer = std::make_unique<PeerConnection>(nodeID, std::stoi(portAddress));
        m_peerConnectionMap[nodeID] =  std::move(peerConnectionPointer) ;
    }
}

void PeerManager::sendMessageToNode(const std::string& nodeID, const std::vector<uint8_t>& inRawByteMessage)
{

   auto it = m_peerConnectionMap.find(nodeID);
    if(it != m_peerConnectionMap.end()) 
    {
        if(it->second->isConnected())
        {

           it->second->sendDataToPeers(inRawByteMessage);
        }
        else
        {
            Logger::getInstance().log(Logger::Level::ERROR, "Node ID " + nodeID + " is not connected");

        }

    }
    else
    {
        Logger::getInstance().log(Logger::Level::ERROR, "Node ID " + nodeID + " is not present in the map");

    }
}

void PeerManager::sendMessageToAllNodes(const std::vector<uint8_t>& inRawByteMessage)
{
    for(const auto& [key , _] : m_peerConnectionMap)
    {

        sendMessageToNode(key, inRawByteMessage);
    }

}

int PeerManager::totalNodesConnected()
{
    int count = 0 ;
    for(const auto& [key , value] : m_peerConnectionMap)
    {
        if(value->isConnected())
        {
            count++;
        }   
    }

    return count ;
}
    