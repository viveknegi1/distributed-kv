#pragma once
#include "peer_connection.h"
#include <string>
#include <unordered_map>
#include <memory>
#include "node_config.h"
#include <vector>

class PeerManager{

    public:
        PeerManager(const NodeConfig& nodeConfig);
        void sendMessageToNode(const std::string& nodeID, const std::string& message);
        void sendMessageToAllNodes(const std::string& message);

    private:
        std::unordered_map<std::string, std::unique_ptr<PeerConnection>> m_peerConnectionMap ;
    
};