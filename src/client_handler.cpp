#include "client_handler.h"
#include "logger.h"
#include "message.h"
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <unistd.h> 

ClientHandler::ClientHandler(RaftNode& inRaftNode , PeerManager& inPeerManager, KvStore& inKvStore, int inPortNumber) : m_raftNode(inRaftNode) , m_peerManager(inPeerManager) , m_kvStore (inKvStore)
{
    m_portNumber = inPortNumber ;
    start();
}

void ClientHandler::start()
{
    if(initialize())
    {
        if(bindSocket())
        {
            if (listen(m_serverSocket, 10) == -1)
            {
                Logger::getInstance().log(Logger::Level::ERROR, "Failed to listen on socket");
                return;
            }
            m_thread = std::thread(&ClientHandler::startListening, this);
        }
    }
}

bool ClientHandler::initialize()
{
    m_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_serverSocket == -1) 
    {
        Logger::getInstance().log(Logger::Level::ERROR, "Failed to create socket");
        return false;
    }
    int opt = 1;
    setsockopt(m_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    return true ;
}

bool ClientHandler::bindSocket()
{
    sockaddr_in in_sockaddrStruct ; 
    in_sockaddrStruct.sin_family = AF_INET ;
    in_sockaddrStruct.sin_port = htons(m_portNumber) ; //converts byte order 
    in_sockaddrStruct.sin_addr.s_addr = INADDR_ANY ;

    int result = bind(m_serverSocket , (sockaddr*)&in_sockaddrStruct, sizeof(in_sockaddrStruct));
    if( result == -1)
    {
        Logger::getInstance().log(Logger::Level::ERROR, "Failed to bind the socket");
        return false;
    }
    return true ;
}

void ClientHandler::startListening()
{
    while(!m_shouldStop)
    {           
        int socketFd = acceptConnection(); 
        if(socketFd >= 0 )
        {
            m_threadPool.push_back( std::thread (&ClientHandler::handleConnection, this, socketFd) );
        } 
    }
}        

int ClientHandler::acceptConnection()
{
    sockaddr_in clientAddr;
    socklen_t clientSize = sizeof(clientAddr);
    int clientFd = accept(m_serverSocket, (sockaddr*)&clientAddr, &clientSize);

    if(clientFd == -1)
    {
        Logger::getInstance().log(Logger::Level::ERROR, "Failed to accept the client");      
    }
    return clientFd;
}

void ClientHandler::handleConnection(int socketFd)
{
 while(true)
    {
        // Receive input from client and parse it
        char buffer[4096]; 
        int bytesReceived = recv(socketFd, buffer, sizeof(buffer), 0); 
        Logger::getInstance().log(Logger::Level::INFO, "Received " + std::to_string(bytesReceived) + " bytes: " + buffer);
        if(bytesReceived == -1)
        {
            Logger::getInstance().log(Logger::Level::ERROR, "Insufficient Data received from client request");
            return ;

        }
        else if (bytesReceived == 0)
        {
            Logger::getInstance().log(Logger::Level::ERROR, "Client Disconnected");
            return ;
        } 
        else
        {
            std::string bufferString(buffer, bytesReceived);
            auto parsedResult = ClientHandler::parse(bufferString);
            std::string response ;
            if(isWriteCommand(parsedResult))
            {
                response = handleWriteCommand(parsedResult);
            }
            else
            {
               response = handleReadCommand(parsedResult); 
            }
         
            // Send response back to client
            ssize_t bytesSent = send(socketFd, response.c_str(), response.size(), 0);
            if(bytesSent == -1)
            {
                Logger::getInstance().log(Logger::Level::ERROR, "Failed to send response to client");
                break;
            }
        }
    }    
}

std::string ClientHandler::handleReadCommand(ClientHandler::ParsedCommand& inParsedCommand)      
{

    if(inParsedCommand.type == ClientHandler::CommandType::GET ) 
    {
        auto argumentList = inParsedCommand.args ;
        if(argumentList.empty())
        {
            Logger::getInstance().log(Logger::Level::ERROR, "Arguments are empty");
            return "NULL\n";         
        }
              
        auto key = argumentList.at(0);
        auto response =  m_kvStore.get(key);
        if ( response.has_value())  
        {
            return response.value() + "\n";
        }
        return "NULL\n";                                 
    }
    else if(inParsedCommand.type == ClientHandler::CommandType::EXISTS)
    {
       auto argumentList = inParsedCommand.args ;        
       if(argumentList.empty())
        {
            Logger::getInstance().log(Logger::Level::ERROR, "Arguments are empty");
            return "NULL\n";
                        
        }
        auto key = argumentList.at(0);
        auto response =  m_kvStore.exists(key);      
        return  (response) ?  "true\n" : "false\n";
    }
    else if (inParsedCommand.type == ClientHandler::CommandType::KEYS) 
    {
        auto keys = m_kvStore.keys();
        std::string response ;
        for(const auto& key : keys)
        {
            response += key + "\n";
        }
        return response;
    }
    else
    {
        Logger::getInstance().log(Logger::Level::ERROR, "Unknown command");
        return "ERROR: unknown command\n";
    }

}

std::string ClientHandler::handleWriteCommand(ClientHandler::ParsedCommand& inParsedCommand)
{
    if(inParsedCommand.type == ClientHandler::CommandType::SET ) 
    {
        auto argumentList = inParsedCommand.args ;
        if(argumentList.empty() || argumentList.size() == 1)
        {
            Logger::getInstance().log(Logger::Level::ERROR, "Arguments are fewer than expected");
            return "NULL\n";
        }

        else
        {
            // If node is the leader directly execute the command  via applyCommand 
            if(m_raftNode.isLeader())
            {
               Command cmd = SetCommand{argumentList.at(0), argumentList.at(1)};
               m_raftNode.applyCommand(cmd);
               return "OK\n";
            }
            
            SetCommand setStruct;
            setStruct.key =  argumentList.at(0);
            setStruct.value = argumentList.at(1);

            ClientWriteData clientWriteDataStruct;
            clientWriteDataStruct.term = m_raftNode.getCurrentTerm();
            clientWriteDataStruct.commandType = setStruct;
            auto rawBytes = MessageSerializer::serializeClientWriteRequest(clientWriteDataStruct);
            m_peerManager.sendMessageToNode(m_raftNode.getCurrentLeaderID(), rawBytes)  ; 
            return "OK\n"; 
        }

    }

    else if (inParsedCommand.type == ClientHandler::CommandType::DEL)
    {
        auto argumentList = inParsedCommand.args ;
        if(argumentList.empty())
        {
            Logger::getInstance().log(Logger::Level::ERROR, "Arguments are fewer than expected");  
            return "NULL\n"; 
        }
        else
        {
            if(m_raftNode.isLeader())
            {
               Command cmd = DeleteCommand{argumentList.at(0)};
               m_raftNode.applyCommand(cmd);
               return "OK\n"; 
            }

            DeleteCommand deleteStruct;
            deleteStruct.key =  argumentList.at(0);
            ClientWriteData clientWriteDataStruct;
            clientWriteDataStruct.term = m_raftNode.getCurrentTerm();
            clientWriteDataStruct.commandType = deleteStruct;

            auto rawBytes = MessageSerializer::serializeClientWriteRequest(clientWriteDataStruct);
            m_peerManager.sendMessageToNode(m_raftNode.getCurrentLeaderID(), rawBytes)  ; 
            return "True\n";
        }
    }
    else if (inParsedCommand.type == ClientHandler::CommandType::FLUSH) 
    {
        if(m_raftNode.isLeader())
            {
               Command cmd = FlushCommand{};
               m_raftNode.applyCommand(cmd); 
               return "OK\n";
            }
        
        FlushCommand flushStruct;
        ClientWriteData clientWriteDataStruct;
        clientWriteDataStruct.term = m_raftNode.getCurrentTerm();

        clientWriteDataStruct.commandType = flushStruct;
        auto rawBytes = MessageSerializer::serializeClientWriteRequest(clientWriteDataStruct);
        m_peerManager.sendMessageToNode(m_raftNode.getCurrentLeaderID(), rawBytes)  ; 
        return "OK\n";
    }
    else
    {
        Logger::getInstance().log(Logger::Level::ERROR, "Unknown command");
        return "ERROR: unknown command\n";
    }

}

ClientHandler::CommandType ClientHandler::stringToCommandType(const std::string& commandString)
{
    if(commandString == "SET")
    { 
        return ClientHandler::CommandType::SET;
    }
    else if(commandString == "GET") 
    {
        return ClientHandler::CommandType::GET;
    }
    else if(commandString == "DEL") 
    {
        return ClientHandler::CommandType::DEL;
    }
    else if(commandString == "EXISTS") 
    {
        return ClientHandler::CommandType::EXISTS;
    }
    else if(commandString == "KEYS") 
    {
        return ClientHandler::CommandType::KEYS;
    }
    else if(commandString == "FLUSH") 
    {
        return ClientHandler::CommandType::FLUSH;
    }
    else 
    {
        return ClientHandler::CommandType::UNKNOWN;
    }
} 

ClientHandler::ParsedCommand ClientHandler::parse(const std::string& rawCommandString)
 {
    ClientHandler::ParsedCommand parsedResult;
    std::string cleanedString = rawCommandString;
    // Raw strings can have '\r\n at the end. So cleaning up them for better parsing
    while (!cleanedString.empty() && (cleanedString.back() == '\n' || cleanedString.back() == '\r')) 
    {
        cleanedString.pop_back();
    }

    bool isOnlySpaces = (cleanedString.find_first_not_of(" \t") == std::string::npos);

    size_t firstSpacePos = cleanedString.find(' ');
    std::string commandString = cleanedString.substr(0, firstSpacePos);
    auto command = stringToCommandType(commandString);

    if (isOnlySpaces) {
        parsedResult.type = command;
        return parsedResult ;
    }


    if(firstSpacePos == std::string::npos) 
    {
        // This command have no arguments. So return early.
        parsedResult.type = command;
        return parsedResult;  
    }

    std::string argumentSubstring = cleanedString.substr(firstSpacePos + 1);
    std::vector<std::string> argumentList;
    std::string currentArgument;
    for( auto& character : argumentSubstring)
    {
        if(character == ' ')
        {
           argumentList.push_back(currentArgument) ;
           currentArgument = "";
        }
        else
        {
            currentArgument += character ;
        }
    }
    // We have recieved final 'space' . Push remaining argument to the vector.
    argumentList.push_back(currentArgument) ;
    parsedResult.type = command ;
    parsedResult.args = argumentList ;

    return parsedResult;
 }

bool ClientHandler::isWriteCommand(ClientHandler::ParsedCommand inCommandType)
{
    if(inCommandType.type == ClientHandler::CommandType::SET || inCommandType.type == ClientHandler::CommandType::DEL || inCommandType.type == ClientHandler::CommandType::FLUSH)
    {
        return true ;
    }
    return false ;
}
    
ClientHandler::~ClientHandler()
{
    m_shouldStop = true;
    if(m_serverSocket != -1)
    {
        close(m_serverSocket);
        m_serverSocket = -1;
    }
    if(m_thread.joinable())
    {
        m_thread.join();
    }
    for(auto& t : m_threadPool)
    {
        if(t.joinable()) t.join();
    }
}    