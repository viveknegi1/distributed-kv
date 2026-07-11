#pragma once

#include <memory>
#include "node_config.h"
#include "peer_connection.h"
#include <string>
#include <unordered_map>

class PeerManager
{
    public:
        PeerManager(const NodeConfig& nodeConfig);
        void sendMessageToNode(const std::string& nodeID, const std::vector<uint8_t>& inRawByteMessage);
        void sendMessageToAllNodes(const std::vector<uint8_t>& inRawByteMessage);
        int totalNodesConnected();

    private:
        std::unordered_map<std::string , std::unique_ptr<PeerConnection>> m_peerConnectionMap ;
            
};