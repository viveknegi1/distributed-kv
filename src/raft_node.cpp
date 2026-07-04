#include "raft_node.h"
#include <random>
#include <chrono>
#include "logger.h"
#include "message.h"



RaftNode::RaftNode(NodeConfig& inNodeConfig , PeerManager& inPeerManager, ReplicationLog& inReplicationLog) : m_nodeConfigObj(inNodeConfig) , m_peerManagerObj(inPeerManager) , m_replicationLogObj(inReplicationLog)
{

}

void RaftNode::start()
{

    m_electionThread = std::thread(&RaftNode::runElectionTimer, this);

}

void RaftNode::runElectionTimer()
{
    while(m_peerManagerObj.totalNodesConnected() < 2)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  // m_peerManagerObj.sendMessageToAllNodes("Total nodes connected are " + std::to_string(m_peerManagerObj.totalNodesConnected()));
   
    while(!m_shouldStop)
    {
        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int> dist(1000, 2000);
        int timeout = dist(rng);
        std::unique_lock<std::mutex> lock(m_mutex);
        bool timedOut = !m_cv.wait_for(lock, std::chrono::milliseconds(timeout),
        [this] { return m_heartbeatReceived.load(); });
        m_heartbeatReceived = false ;
        if(timedOut && m_state ==  RaftNode::NodeState::Follower)
        {   
            startElection();
        }
    }   
}

void RaftNode::startElection()
{
    Logger::getInstance().log(Logger::Level::INFO, "startElection called, lastLogIndex: " + 
    std::to_string(m_replicationLogObj.getLastAppendedIndex()));
    if(!m_votedFor.empty())  // already voted this term — don't start election
    {
        return;
    }   
    m_currentTerm++;
    m_state =  RaftNode::NodeState::Candidate;
    auto ownNodeID = m_nodeConfigObj.getOwnNodeID();
    m_votedFor = ownNodeID ;
    
        Logger::getInstance().log(Logger::Level::INFO, "Election has started from node Id " + ownNodeID);
        m_voteCount = 1;
        VoteRequestData voteRequestData ;
        voteRequestData.candidateID = m_nodeConfigObj.getOwnNodeID();
        voteRequestData.term = m_currentTerm;
        voteRequestData.lastLogIndex = m_replicationLogObj.getLastAppendedIndex();
        auto rawBytes = MessageSerializer::serializeVoteRequest(voteRequestData);
        m_peerManagerObj.sendMessageToAllNodes(rawBytes);

        Logger::getInstance().log(Logger::Level::INFO, "startElection completed");
    

}


void RaftNode::becomeFollower()
{
    if(m_state != NodeState::Follower)
    {
        m_state = NodeState::Follower;
        m_voteCount = 0;
        Logger::getInstance().log(Logger::Level::INFO, "Became a follower");
    }
}


void RaftNode::becomeLeader()
{

    m_state =  RaftNode::NodeState::Leader;
    m_voteCount = 0 ;
    Logger::getInstance().log(Logger::Level::INFO, "Became a leader");
    m_heartBeatThread = std::thread(&RaftNode::sendHeartbeats, this);

}

void RaftNode::sendHeartbeats()
{

    while(!m_shouldStop && m_state ==  RaftNode::NodeState::Leader)
    {
        HeartBeatData heartBeatData ;
        heartBeatData.leaderID = m_nodeConfigObj.getOwnNodeID();
        heartBeatData.term = m_currentTerm;
        auto rawBytes = MessageSerializer::serializeHeartbeat(heartBeatData);
        m_peerManagerObj.sendMessageToAllNodes(rawBytes);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

}

void RaftNode::receiveVote(const int inTermCount, const bool inGranted)
{
    if(inTermCount > m_currentTerm)
    {
        m_currentTerm = inTermCount;
        becomeFollower();
        return;
    }
    
    if(inGranted)
    {
        m_voteCount++;
        if(m_voteCount >= 2) 
        {
            becomeLeader();
        }
    }
}

void RaftNode::receiveHeartBeat(const int term , const std::string& inLeaderId)
{
    m_currentLeaderID = inLeaderId;
    if( term > m_currentTerm)
    {
        m_currentTerm = term ;
        m_votedFor = "" ;
    }
    becomeFollower();
    m_heartbeatReceived = true;
    m_cv.notify_one();
}

void RaftNode::requestVote(const std::string& inCandidateId, int termCount, int lastLoggedIndex)
{
    VoteResponseData response;
    response.term = m_currentTerm;
    
    if(m_votedFor.empty() && termCount >= m_currentTerm)
    {
        m_votedFor = inCandidateId;
        response.granted = true;
        Logger::getInstance().log(Logger::Level::INFO, "Vote granted for " + inCandidateId);
    }
    else
    {
        response.granted = false;
        Logger::getInstance().log(Logger::Level::INFO, "Vote denied for " + inCandidateId);
    }
    
    auto rawBytes = MessageSerializer::serializeVoteResponse(response);
    m_peerManagerObj.sendMessageToNode(inCandidateId, rawBytes);
}

RaftNode::~RaftNode()
{
    m_shouldStop = true ;
    if (m_electionThread.joinable())
    {
        m_electionThread.join();
    }
    if (m_heartBeatThread.joinable())
    {
        m_heartBeatThread.join();
    }
}


