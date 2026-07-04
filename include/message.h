#pragma once
#include <string>
#include <variant>
#include <vector>

    struct SetCommand
    {
        std::string key ;
        std::string value ;
    };

     struct DeleteCommand
    {
        std::string key ;
    };

    struct FlushCommand
    {

    };

    using Command = std::variant<SetCommand, DeleteCommand, FlushCommand>;

    struct LogEntry
    {
        Command commandType;
        int term;
        int index;

    };

   enum class MessageType : uint8_t 
   {
     VOTE_REQUEST  = 1,
     VOTE_RESPONSE = 2,
     HEARTBEAT     = 3
   };

   struct VoteRequestData
   {
    int term;
    std::string candidateID;
    int lastLogIndex ;

   };

   struct VoteResponseData
   {
    int term ;
    bool granted ;

   };

   struct HeartBeatData
   {

    int term;
    std::string leaderID;

   };

    namespace MessageSerializer 
    {
        std::vector<uint8_t> serializeVoteRequest(const VoteRequestData& inVoteRequestData);
        VoteRequestData deserializeVoteRequest(std::vector<uint8_t> inPutData);

        std::vector<uint8_t> serializeVoteResponse(const VoteResponseData& inVoteResponseData);
        VoteResponseData deserializeVoteResponse(std::vector<uint8_t> inPutData);

        std::vector<uint8_t> serializeHeartbeat(const HeartBeatData& inHeartBeatData);
        HeartBeatData deserializeHeartbeat(std::vector<uint8_t> inPutData);
    
    }