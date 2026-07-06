#pragma once
#include <string>
#include <vector>

class NodeConfig{

    public:
      struct PeerStruct
        {
            std::string nodeID ;
            std::string portAddress ;
        };

        NodeConfig(std::string inNodeID , const std::string& inNodeConfigPath);    
        std::string getAddressOFNode(const std::string& inNodeID) const ;
        std::string getNodeIdOnPortAddress(const std::string& inPortAddress) const;
        std::vector<PeerStruct> getAllOtherNodes() const ;
        std::string getOwnNodeID() const ;

    private:
        std::string m_nodeId ; // our nodeID
        std::string m_portAddress ; // our portAddress 
        std::vector<PeerStruct> m_nodesList ;
        std::string m_configPath ; 
        void readNodeConfig();
    
};