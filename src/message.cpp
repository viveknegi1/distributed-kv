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
        
        // last four bytes will be last log index 
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

    std::vector<uint8_t> serializeClientWriteRequest(const ClientWriteData& inClientWriteData)
    {    
        std::vector<uint8_t> buffer;
        //first byte should be type
        buffer.push_back(static_cast<uint8_t>(MessageType::CLIENT_WRITE));

        //second till fifth bytes is term 
        uint32_t term = inClientWriteData.term;
        size_t offset = buffer.size();
        buffer.resize(offset + sizeof(term)); 
        std::memcpy(buffer.data() + offset, &term, sizeof(term));

        auto command = inClientWriteData.commandType ;
        std::visit([&buffer](const auto& cmd) 
        {
            using T = std::decay_t<decltype(cmd)>;
            if constexpr (std::is_same_v<T, SetCommand>) 
            {
                buffer.push_back(1); // type byte for SET
                // serialize cmd.key and cmd.value
                auto key = cmd.key;
                buffer.push_back(static_cast<uint8_t>(key.size()));
                for(const auto c : key)
                {
                    buffer.push_back(c);
                }
                auto value = cmd.value;
                buffer.push_back(static_cast<uint8_t>(value.size()));
                for(const auto c : value)
                {
                    buffer.push_back(c);
                }
            }
            else if constexpr (std::is_same_v<T, DeleteCommand>) 
            {
                buffer.push_back(2); // type byte for DEL
                auto key = cmd.key;
                buffer.push_back(static_cast<uint8_t>(key.size()));
                for(const auto c : key)
                {
                    buffer.push_back(c);
                }
            }
            else if constexpr (std::is_same_v<T, FlushCommand>) 
            {
                buffer.push_back(3); // type byte for FLUSH
            }
        }, command);   
        return buffer ;
    }

    ClientWriteData deserializeClientWriteRequest(std::vector<uint8_t> inPutData)
    {
        if(inPutData.size() < 6)
        {
            Logger::getInstance().log(Logger::Level::ERROR, 
            "Client Write buffer too small: " + std::to_string(inPutData.size()));
            return {};
        }

        //Second to fifth byte  will be term
        size_t index = 0;
        int term ;
        std::memcpy(&term, &inPutData[0], sizeof(term));
        index += sizeof(term);

        // next byte will tell the type of request. 
        int nextByte = inPutData.at(index);
        index++; // Advance past the request type byte
        Command currentCommandtype;
        if(nextByte == 1)
        {
            SetCommand currentCommand;
    
            // 1. Read key length
            int keyLength = inPutData.at(index);
            index++; // Advance past the key length byte so index points to the start of the string
    
            // 2. Read key string
            std::string key;
            for(int i = 0; i < keyLength; i++)
            {
                key += inPutData[index++]; // Reads string data and moves index forward
            }
            
            currentCommand.key = key ;
            // 3. Read value length
            int valueLength = inPutData.at(index);
            index++; // Advance past the value length byte
    
            // 4. Read value string
            std::string value;
            for(int i = 0; i < valueLength; i++)
            {
                value += inPutData[index++]; // Reads string data and moves index forward
            }  
            currentCommand.value = value ;  
            currentCommandtype = currentCommand;
        }
        else if(nextByte == 2)
        {
            DeleteCommand currentCommand;
            // 1. Read key length
            int keyLength = inPutData.at(index);
            index++; // Advance past the key length byte so index points to the start of the string
    
            // 2. Read key string
            std::string key;
            for(int i = 0; i < keyLength; i++)
            {
                key += inPutData[index++]; // Reads string data and moves index forward
            }
            currentCommand.key = key;
            currentCommandtype = currentCommand ;

        }
        else
        {
            FlushCommand currentCommand ;
            currentCommandtype = currentCommand ;
        }
        ClientWriteData result ;
        result.term = term ;
        result.commandType = currentCommandtype;

        return result ;
    }

    std::vector<uint8_t> serializeClientWriteResponse(const ClientResponseData& inClientResponseData)
    {
         std::vector<uint8_t> buffer;
        //first byte should be type
        buffer.push_back(static_cast<uint8_t>(MessageType::CLIENT_WRITE_RESPONSE));

        //second till fifth bytes is term 
        uint32_t term = inClientResponseData.term;
        size_t offset = buffer.size();
        buffer.resize(offset + sizeof(term)); 
        std::memcpy(buffer.data() + offset, &term, sizeof(term));

        //sixth byte is a bool
        bool wasSucessful = inClientResponseData.wasSucessful;
        buffer.push_back(static_cast<uint8_t>(wasSucessful));

        return buffer ;
    }


    ClientResponseData deserializeClientWriteResponse(std::vector<uint8_t> inPutData)
    {
        if(inPutData.size() < 5)
        {
            Logger::getInstance().log(Logger::Level::ERROR, 
            "Client Response buffer too small: " + std::to_string(inPutData.size()));
            return {};
        }       
        //Second to fifth byte  will be term
        int term ;
        std::memcpy(&term, &inPutData[0], sizeof(term));
        bool wasSucessful = inPutData.at(4);
        
        ClientResponseData result ;
        result.term = term ;
        result.wasSucessful = wasSucessful ;
        return result ;
    }

    std::vector<uint8_t> serializeAppendEntries(const AppendEntriesData& inAppendEntriesData)
    {
        std::vector<uint8_t> buffer;
        //first byte should be type
        buffer.push_back(static_cast<uint8_t>(MessageType::APPEND_ENTRIES));

        //second till fifth bytes is term 
        uint32_t term = inAppendEntriesData.term;
        size_t offset = buffer.size();
        buffer.resize(offset + sizeof(term)); 
        std::memcpy(buffer.data() + offset, &term, sizeof(term));

        auto command = inAppendEntriesData.commandType ;
        std::visit([&buffer](const auto& cmd) 
        {
            using T = std::decay_t<decltype(cmd)>;
            if constexpr (std::is_same_v<T, SetCommand>) 
            {
                buffer.push_back(1); // type byte for SET
                // serialize cmd.key and cmd.value
                auto key = cmd.key;
                buffer.push_back(static_cast<uint8_t>(key.size()));
                for(const auto c : key)
                {
                    buffer.push_back(c);
                }
                auto value = cmd.value;
                buffer.push_back(static_cast<uint8_t>(value.size()));
                for(const auto c : value)
                {
                    buffer.push_back(c);
                }
            }
            else if constexpr (std::is_same_v<T, DeleteCommand>) 
            {
                buffer.push_back(2); // type byte for DEL
                auto key = cmd.key;
                buffer.push_back(static_cast<uint8_t>(key.size()));
                for(const auto c : key)
                {
                    buffer.push_back(c);
                }
            }
            else if constexpr (std::is_same_v<T, FlushCommand>) 
            {
                buffer.push_back(3); // type byte for FLUSH
            }
        }, command); 
         
        // last 4 byte should be last log index
        uint32_t logIndex = inAppendEntriesData.logIndex ;
        offset = buffer.size();
        buffer.resize(offset + sizeof(logIndex)); 
        std::memcpy(buffer.data() + offset, &logIndex, sizeof(logIndex));
  
        return buffer ;
    }

    AppendEntriesData deserializeAppendEntries(std::vector<uint8_t> inPutData)
    {
        if(inPutData.size() < 6)
        {
            Logger::getInstance().log(Logger::Level::ERROR, 
            "Client Write buffer too small: " + std::to_string(inPutData.size()));
            return {};
        }

        //Second to fifth byte  will be term
        size_t index = 0;
        int term ;
        std::memcpy(&term, &inPutData[0], sizeof(term));
        index += sizeof(term);

        // next byte will tell the type of request. 
        int nextByte = inPutData.at(index);
        index++; // Advance past the request type byte
        Command currentCommandtype;
        if(nextByte == 1)
        {
            SetCommand currentCommand;
    
            // 1. Read key length
            int keyLength = inPutData.at(index);
            index++; // Advance past the key length byte so index points to the start of the string
    
            // 2. Read key string
            std::string key;
            for(int i = 0; i < keyLength; i++)
            {
                key += inPutData[index++]; // Reads string data and moves index forward
            }
            
            currentCommand.key = key ;
            // 3. Read value length
            int valueLength = inPutData.at(index);
            index++; // Advance past the value length byte
    
            // 4. Read value string
            std::string value;
            for(int i = 0; i < valueLength; i++)
            {
                value += inPutData[index++]; // Reads string data and moves index forward
            }  
            currentCommand.value = value ;  
            currentCommandtype = currentCommand;
        }
        else if(nextByte == 2)
        {
            DeleteCommand currentCommand;
            // 1. Read key length
            int keyLength = inPutData.at(index);
            index++; // Advance past the key length byte so index points to the start of the string
    
            // 2. Read key string
            std::string key;
            for(int i = 0; i < keyLength; i++)
            {
                key += inPutData[index++]; // Reads string data and moves index forward
            }
            currentCommand.key = key;
            currentCommandtype = currentCommand ;

        }
        else
        {
            FlushCommand currentCommand ;
            index++;
            currentCommandtype = currentCommand ;
        }

         AppendEntriesData result ;
        //last entery will be logIndex;
        int logIndex ;
        std::memcpy(&logIndex, &inPutData[inPutData.size() - 5], sizeof(logIndex));

        result.logIndex = logIndex;
        result.term = term ;
        result.commandType = currentCommandtype;

        return result ;
    }
    
}