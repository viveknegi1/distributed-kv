#pragma once
#include "message.h"
#include <vector>
#include <mutex>
#include <optional>

class ReplicationLog
{
    private:
        std::vector<LogEntry> m_logs;
        int m_lastAppendedIndex = -1 ;
        int m_commitIndex = -1 ;
        mutable std::mutex m_mutex ;  

    public:
      void appendEntry(LogEntry inLogEntry);
      std::optional<LogEntry> getEntryByIndex(int inIndex) const;
      int getCommitIndex() const;
      void updateCommitIndex(int inCommitIndex) ;
      int getLastAppendedIndex() const;

};