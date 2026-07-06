#include "message.h"
#include "logger.h"


namespace MessageSerializer 
{
    std::vector<uint8_t> serializeVoteRequest(const VoteRequestData& inVoteRequestData)
    {
            std::vector<uint8_t> buffer;
        //first byte should be type
        buffer.push_back(static_cast<uint8_t>(MessageType::VOTE_REQUEST));

        //second till fifth bytes is term 
        uint32_t term = inVoteRequestData.term;
        size_t offset = buffer.size();
        buffer.resize(offset + sizeof(term)); 
        std::memcpy(buffer.data() + offset, &term, sizeof(term));

        //sixth byte is length of the string
        std::string candidateID = inVoteRequestData.candidateID;
        buffer.push_back(static_cast<uint8_t>(candidateID.size()));

        //Seventh byte onwards is string.
        for(const auto c : candidateID)
        {
            buffer.push_back(c);
        }

        // last 4 byte should be last log index
        uint32_t lastLogIndex = inVoteRequestData.lastLogIndex ;
        offset = buffer.size();
        buffer.resize(offset + sizeof(term)); 
        std::memcpy(buffer.data() + offset, &lastLogIndex, sizeof(lastLogIndex));
        
        return buffer ;
    }

    VoteRequestData deserializeVoteRequest(std::vector<uint8_t> inPutData)
    {

        if(inPutData.size() < 5)
        {
            Logger::getInstance().log(Logger::Level::ERROR, 
            "VoteRequest buffer too small: " + std::to_string(inPutData.size()));
            return {};
        }
        
        //First to fourth byte  will be term
        int term ;
        std::memcpy(&term, &inPutData[0], sizeof(term));
        int stringLength = inPutData.at(4);
        std::string candidateID;
        for(int i = 5  ; i < stringLength + 5 ; i++)
        {
            candidateID += inPutData[i];
        }

        VoteRequestData result ;
        result.term = term ;
        result.candidateID = candidateID ;
        
        // last for bytes will be last log index 
        int lastLogIndex ;
        std::memcpy(&lastLogIndex, &inPutData[inPutData.size() - 5], sizeof(lastLogIndex));
        result.lastLogIndex = lastLogIndex;

        return result ;
    }

    std::vector<uint8_t> serializeVoteResponse(const VoteResponseData& inVoteResponseData)
    {

        std::vector<uint8_t> buffer;
        //first byte should be type
        buffer.push_back(static_cast<uint8_t>(MessageType::VOTE_RESPONSE));

        //second till fifth bytes is term 
        uint32_t term = inVoteResponseData.term;
        size_t offset = buffer.size();
        buffer.resize(offset + sizeof(term)); 
        std::memcpy(buffer.data() + offset, &term, sizeof(term));

        //sixth byte is a bool
        bool voteGranted = inVoteResponseData.granted;
        buffer.push_back(static_cast<uint8_t>(voteGranted));

        return buffer ;
    }

    VoteResponseData deserializeVoteResponse(std::vector<uint8_t> inPutData)
    {
        if(inPutData.size() < 5)
        {
            Logger::getInstance().log(Logger::Level::ERROR, 
            "VoteResponse buffer too small: " + std::to_string(inPutData.size()));
            return {};
        }
        
        //Second to fifth byte  will be term
        int term ;
        std::memcpy(&term, &inPutData[0], sizeof(term));
        bool voteGranted = inPutData.at(4);
        
        VoteResponseData result ;
        result.term = term ;
        result.granted = voteGranted ;

        return result ;
    }

    std::vector<uint8_t> serializeHeartbeat(const HeartBeatData& inHeartBeatData)
    {
        std::vector<uint8_t> buffer;
        //first byte should be type
        buffer.push_back(static_cast<uint8_t>(MessageType::HEARTBEAT));

        //second till fifth bytes is term 
        uint32_t term = inHeartBeatData.term;
        size_t offset = buffer.size();
        buffer.resize(offset + sizeof(term)); 
        std::memcpy(buffer.data() + offset, &term, sizeof(term));

        //sixth byte is length of the string
        std::string leaderID = inHeartBeatData.leaderID;
        buffer.push_back(static_cast<uint8_t>(leaderID.size()));

        //Seventh byte onwards is string.
        for(const auto c : leaderID)
        {
            buffer.push_back(c);
        }

        return buffer ;
    }

    HeartBeatData deserializeHeartbeat(std::vector<uint8_t> inPutData)
    { 
        if(inPutData.size() < 5)
        {
            Logger::getInstance().log(Logger::Level::ERROR, 
            "HeartBeat buffer too small: " + std::to_string(inPutData.size()));
            return {};
        }

        //Second to fifth byte  will be term
        int term ;
        std::memcpy(&term, &inPutData[0], sizeof(term));
        int stringLength = inPutData.at(4);
        std::string leaderID;
        for(int i = 5  ; i < stringLength + 5 ; i++)
        {
            leaderID += inPutData[i];
        }

        HeartBeatData result ;
        result.term = term ;
        result.leaderID = leaderID ;

        return result ;
    }
    
}