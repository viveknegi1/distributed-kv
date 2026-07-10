#pragma once
#include <string>
#include <vector>

class NodeConfig{

    public:
      struct PeerStruct
        {
            std::string nodeID ;
            std::string portAddress ;
            std::string clientPortAddress;
        };

        NodeConfig(std::string inNodeID , const std::string& inNodeConfigPath);    
        std::string getAddressOFNode(const std::string& inNodeID) const ;
        std::string getClientPortOfNode(const std::string& inNodeID) const ;
        std::string getNodeIdOnPortAddress(const std::string& inPortAddress) const;
        std::vector<PeerStruct> getAllOtherNodes() const ;
        std::string getOwnNodeID() const ;

    private:
        std::string m_nodeId ; // our nodeID
        std::string m_portAddress ; // our portAddress 
        std::string m_clientPortAddress ; // port used for communication with client handler.
        std::vector<PeerStruct> m_nodesList ;
        std::string m_configPath ; 
        void readNodeConfig();
    
};