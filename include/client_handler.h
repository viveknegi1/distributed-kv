#pragma once

#include "kv_store.h"
#include "raft_node.h"
#include "peer_manager.h"
#include <thread>
#include <atomic>
#include <vector>

class ClientHandler{

    private:

        enum class CommandType 
        {
            SET,   // internally 0
            GET,   // internally 1
            DEL,    // internally 2
            EXISTS, // internally 3
            KEYS, // internally 4
            FLUSH, // internally 5
            UNKNOWN   // internally 6
        };

        struct ParsedCommand 
        {
            CommandType type;
            std::vector<std::string> args;
         } ;

        RaftNode& m_raftNode;
        PeerManager& m_peerManager;
        KvStore& m_kvStore;

        int m_portNumber = -1 ;
        int m_serverSocket = -1;
        std::thread m_thread ;
        std::vector<std::thread> m_threadPool ;
        std::atomic<bool> m_shouldStop = false;

        void start();
        bool initialize();
        bool bindSocket();
        void startListening();
        int acceptConnection();
        void handleConnection(int socketFd);
        ClientHandler::ParsedCommand parse(const std::string& rawCommandString);
        ClientHandler::CommandType stringToCommandType(const std::string& commandString);
        std::string handleReadCommand(ParsedCommand& inParsedCommand);
        std::string handleWriteCommand(ParsedCommand& inParsedCommand);
        bool isWriteCommand(ClientHandler::ParsedCommand inCommandType);

    public:
        ClientHandler(RaftNode& inRaftNode , PeerManager& inPeerManager, KvStore& inKvStore, int inPortNumber);
        ~ClientHandler();
        
};