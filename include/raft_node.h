#pragma once
#include <string>
#include "node_config.h"
#include "peer_manager.h"
#include <thread>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include "replication_log.h"

class RaftNode{

    private:
        enum NodeState
        {
            Leader,  //Defaults to 0 
            Candidate, //Defaults to 1
            Follower //Defaults to 2
        };

        int m_currentTerm = 0 ;
        std::string m_votedFor;
        std::string m_currentLeaderID;
        int m_voteCount = 0; 
        NodeState m_state = NodeState::Follower;

        NodeConfig& m_nodeConfigObj;
        PeerManager& m_peerManagerObj;
        ReplicationLog& m_replicationLogObj;

        std::thread m_electionThread;
        std::thread m_heartBeatThread;
        std::atomic<bool> m_shouldStop = false ;

        void startElection(); 
        void becomeLeader(); 
        void becomeFollower(); 
        void runElectionTimer(); 
        void sendHeartbeats() ; 

        std::mutex m_timerMutex ;
        std::condition_variable m_cv ;
        std::atomic<bool> m_heartbeatReceived = false  ; 
        std::mutex m_mutex;

    public:
        void start();
        void requestVote(const std::string& inCandidateId /* send own id*/, int termCount , int lastLoggedIndex);
        void receiveVote(const int inTermCount, const bool inGranted);
        void receiveHeartBeat(const int term , const std::string& inLeaderId);

        RaftNode(NodeConfig& inNodeConfig , PeerManager& inPeerManager, ReplicationLog& inReplicationLog);
        ~RaftNode();

};