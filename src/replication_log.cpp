#include "replication_log.h"
#include "message.h"

void ReplicationLog::appendEntry(LogEntry inLogEntry)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_logs.push_back(inLogEntry);
        m_lastAppendedIndex++;
    }
}

std::optional<LogEntry> ReplicationLog::getEntryByIndex(int inIndex) const
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if(inIndex >= 0 && m_logs.size() > size_t(inIndex))
        {
            return m_logs[inIndex];
        }
    }
    return std::nullopt;
}
      
int ReplicationLog::getCommitIndex() const
{
    return m_commitIndex ;
}
      
void ReplicationLog::updateCommitIndex(int inCommitIndex)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex); 
        m_commitIndex = inCommitIndex ;
    }
}

int ReplicationLog::getLastAppendedIndex() const
{
    return m_lastAppendedIndex ;

}

