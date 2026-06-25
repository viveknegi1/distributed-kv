#pragma once

#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

class KvStore{

private:

    std::unordered_map<std::string, std::string> m_store;
    mutable std::mutex m_mutex;
    
public:

    void set(std::string key, std::string value );
    std::optional<std::string> get(const std::string& key) const;
    bool del(const std::string& key) ; 
    bool exists(const std::string& key) const ;
    std::vector<std::string> keys() const ;
    void flush() ;
    std::unordered_map<std::string, std::string> getSnapShot() const;
    void restoreEntry(const std::string& inComingKey, const std::string& inComingValue) ;
    
}; 