#include <iostream>
#include "node_config.h"
#include "logger.h"

int main(int argc, char* argv[])
{

     if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <your_argument>" << std::endl;
        Logger::getInstance().log(Logger::Level::INFO, std::string ("Usage: ") + argv[0] + " <your_arguments>");
        return 1; 
    }
    Logger::getInstance().log(Logger::Level::INFO, std::string ("Your node id is: ") + argv[1]);
    std::string nodeId = argv[1];
    NodeConfig nodeConfigObj(nodeId , "node.config");
    std::string ourPortAddress =  nodeConfigObj.getAddressOFNode(nodeId);
    Logger::getInstance().log(Logger::Level::INFO, "Our port address is : " + ourPortAddress);
    auto allNodes = nodeConfigObj.getAllOtherNodes();
    for(const auto& node : allNodes)
    {

        Logger::getInstance().log(Logger::Level::INFO, "Peer node:  " + node.nodeID + " port address " + node.portAddress);

    }

    return 0 ;

}