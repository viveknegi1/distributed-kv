#include "node_config.h"
#include <fstream>
#include "logger.h"
#include <string>

namespace
{
    std::string trim(const std::string& str) 
    {
        size_t first = str.find_first_not_of(" \t");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \t");
        return str.substr(first, (last - first + 1));
    }    
}

NodeConfig::NodeConfig(std::string inNodeID , const std::string& inNodeConfigPath)
{
    m_nodeId = inNodeID ;
    m_configPath = inNodeConfigPath ;
    readNodeConfig();
}

std::string NodeConfig::getAddressOFNode(const std::string& inNodeID) const 
{
    std::string resultPortAddress;
    for(const auto& node :m_nodesList )
    {
        if(node.nodeID == inNodeID)
        {
            resultPortAddress = node.portAddress ;
        }
    }

    return resultPortAddress ;

}

std::string NodeConfig::getClientPortOfNode(const std::string& inNodeID) const 
{
    std::string resultClientPortAddress;
    for(const auto& node :m_nodesList )
    {
        if(node.nodeID == inNodeID)
        {
            resultClientPortAddress = node.clientPortAddress ;
        }
    }
    return resultClientPortAddress ;
}

std::string NodeConfig::getNodeIdOnPortAddress(const std::string& inPortAddress) const
{
    std::string resultNodeID;
    for(const auto& node :m_nodesList )
    {
        if(node.portAddress == inPortAddress)
        {
            resultNodeID = node.nodeID ;
        }
    }
    return resultNodeID ;
}

std::vector<NodeConfig::PeerStruct> NodeConfig::getAllOtherNodes()  const
{
    std::vector<NodeConfig::PeerStruct> allOtherNodes ;
    for(const auto& node :m_nodesList )
    {
        if(node.nodeID != m_nodeId)
        {
            allOtherNodes.push_back(node);
        }
    }
    return allOtherNodes ;
}
    
void NodeConfig::readNodeConfig()
{
    std::ifstream file(m_configPath);
    if(!file.is_open())
    {
        Logger::getInstance().log(Logger::Level::ERROR, "Could not open config file");
        return ;
    }
    std::string line;
    int i = 0 ;
    while(std::getline(file, line)) 
    {
        size_t firstEq = line.find('=');
        if (firstEq == std::string::npos)
        {
            Logger::getInstance().log(Logger::Level::ERROR, "No nodeID present at line " + std::to_string(i));
            i++ ;
            continue;
        }
        // Find the second occurrence starting just after the first one
        size_t secondEq = line.find('=', firstEq + 1);
        if (secondEq == std::string::npos) 
        {
            Logger::getInstance().log(Logger::Level::ERROR, "No port address present at line " + std::to_string(i));
            i++;
            continue ;
        } 
        // Find the third occurrence starting just after the second one
        size_t thirdEq = line.find('=', secondEq + 1);
        if (thirdEq == std::string::npos) 
        {
            Logger::getInstance().log(Logger::Level::ERROR, "No client port address present at line " + std::to_string(i));
            i++;
            continue ;
        } 
        // Find the comma that marks the end of the first value
        size_t firstComma = line.find(',', firstEq + 1);

        std::string nodeId, portAddress, clientPortAddress;
            // Extract first value (between first '=' and the comma)
        if (firstComma != std::string::npos && firstComma > firstEq) 
        {
            nodeId = trim(line.substr(firstEq + 1, firstComma - (firstEq + 1)));
        }

        // Find the second comma that marks the end of the second value
         size_t secondComma = line.find(',', secondEq + 1);

        // Extract second value (between second '=' and the second comma)
         if (secondComma != std::string::npos && secondComma > secondEq) 
        {
            portAddress = trim(line.substr(secondEq + 1, secondComma - (secondEq + 1)));
        }

        // Extract third value (everything after the third '=')
        clientPortAddress = trim(line.substr(thirdEq + 1));

        NodeConfig::PeerStruct currentNode ;
        currentNode.nodeID = nodeId ;
        currentNode.portAddress = portAddress ; 
        currentNode.clientPortAddress =  clientPortAddress ;
        if(currentNode.nodeID == m_nodeId)
        {
            m_portAddress = currentNode.portAddress ;
            m_clientPortAddress = clientPortAddress ;
        }      
        m_nodesList.push_back(currentNode);
    }
}
    
std::string NodeConfig::getOwnNodeID() const
{
    return m_nodeId;
}
