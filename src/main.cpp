#include <iostream>
#include "node_config.h"
#include "logger.h"
#include "peer_manager.h"
#include <chrono>
#include <thread>
#include "rpc_server.h"
#include <message.h>
#include <replication_log.h>

int main(int argc, char* argv[])
{

     if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <your_argument>" << std::endl;
        Logger::getInstance().log(Logger::Level::INFO, std::string ("Usage: ") + argv[0] + " <your_arguments>");
        return 1; 
    }
    Logger::getInstance().log(Logger::Level::INFO, std::string ("Our node id is: ") + argv[1]);
    std::string nodeId = argv[1];
    NodeConfig nodeConfigObj(nodeId , "node.config");
    std::string ourPortAddress =  nodeConfigObj.getAddressOFNode(nodeId);
    Logger::getInstance().log(Logger::Level::INFO, "Our port address is : " + ourPortAddress);
     /*
    auto allNodes = nodeConfigObj.getAllOtherNodes();
    for(const auto& node : allNodes)
    {

        Logger::getInstance().log(Logger::Level::INFO, "Peer node:  " + node.nodeID + " port address " + node.portAddress);

    }
   
    PeerManager peerManagerObj(nodeConfigObj);
    auto portAddressString = nodeConfigObj.getAddressOFNode(nodeId);
    auto portAddress = std::stoi(portAddressString);
    RpcServer rpcServerObj(portAddress);

    while(true)
    {
       std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    */
    LogEntry entry;
    entry.term = 1;
    entry.index = 0;
    entry.commandType = SetCommand{"name", "John"};

// Reading back:
    if (std::holds_alternative<SetCommand>(entry.commandType)) {

        auto& cmd = std::get<SetCommand>(entry.commandType);
        Logger::getInstance().log(Logger::Level::INFO, "Command key : " + cmd.key + " Command value:" + cmd.value );
    // cmd.key, cmd.value are accessible
    }
    return 0 ;

}