#include "logger.h"

#include <ctime>
#include <iostream>

Logger::Logger() 
{
    m_logFile.open("server.log", std::ios::app);  // append mode
}

Logger& Logger::getInstance()
{
    static Logger instance ;
    return instance;
}

void Logger::log(Logger::Level level , const std::string& message)
{
    auto now = std::time(nullptr);
    auto timeStr = std::string(std::ctime(&now));
    timeStr.pop_back();  // remove trailing newline that ctime adds
    std::string loglevel = levelToString(level);
    {
       std::unique_lock<std::mutex> lock(m_mutex);
       m_logFile<< "[ "<<timeStr << " ] " << "[ " << loglevel << " ] " << message << "\n";
       std::cout<< "[ "<<timeStr << " ] " << "[ " << loglevel << " ] " << message << "\n";
       m_logFile.flush();  // force write to disk
    }
  
}

std::string Logger::levelToString(Logger::Level level)
{
    switch(level) 
    {
        case Level::INFO:  return "INFO";
        case Level::WARN:  return "WARN";
        case Level::ERROR: return "ERROR";
        default:           return "UNKNOWN";
    }
}