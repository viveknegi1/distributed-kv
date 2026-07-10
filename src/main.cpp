#include <iostream>
#include "node_config.h"
#include "logger.h"
#include "peer_manager.h"
#include <chrono>
#include <thread>
#include "rpc_server.h"
#include <message.h>
#include <replication_log.h>
#include "raft_node.h"
#include <signal.h>
#include "kv_store.h"
#include "client_handler.h"

int main(int argc, char* argv[])
{

    signal(SIGPIPE, SIG_IGN); 
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
    
    PeerManager peerManagerObj(nodeConfigObj);
    
    auto portAddressString = nodeConfigObj.getAddressOFNode(nodeId);
    auto portAddress = std::stoi(portAddressString);
    RpcServer rpcServerObj(portAddress);
    ReplicationLog replicationLogObj ;
    KvStore kvStoreObj ;
    RaftNode raftNodeObj(nodeConfigObj, peerManagerObj, replicationLogObj, kvStoreObj);
    auto raftNodePtr = &raftNodeObj;
    rpcServerObj.setRaftNode(raftNodePtr);
    auto clientPortString = nodeConfigObj.getClientPortOfNode(nodeId);
    auto clientPort = std::stoi(clientPortString);
    ClientHandler clientHandler(raftNodeObj, peerManagerObj, kvStoreObj, clientPort);
    raftNodeObj.start();

    while(true)
    {
       std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    

    return 0 ;

}