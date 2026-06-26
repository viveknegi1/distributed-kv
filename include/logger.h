#pragma once

#include <fstream>
#include <mutex>
#include <string>

class Logger {

public:
 

    enum class Level {
    INFO,   // internally 0
    WARN,   // internally 1
    ERROR   // internally 2
};
    static Logger& getInstance();
    void log(Level level , const std::string& message);

private:
        std::mutex m_mutex;
        std::ofstream m_logFile;
        static std::string levelToString(Level level);
        Logger();


Logger(const Logger&) = delete;
Logger& operator=(const Logger&) = delete;

};