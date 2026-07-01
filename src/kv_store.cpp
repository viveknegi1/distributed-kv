#include "kv_store.h"

void KvStore::set(std::string key, std::string value)
{
    {   
        std::lock_guard<std::mutex> lock(m_mutex);
        m_store[std::move(key)] = std::move(value);
    }       
}
   
std::optional<std::string> KvStore::get(const std::string& key) const
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_store.find(key);
        if( it != m_store.end()) 
        {    
            return it->second;    
        }
    }
    return std::nullopt; 
} 

bool KvStore::del(const std::string& key)
{
    size_t result ;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        result = m_store.erase(key);
            
    }
    return result > 0; 
}  
    
bool KvStore::exists(const std::string& key) const
{
    bool keyExists = false ;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_store.find(key);
        if(it != m_store.end()) {
            keyExists = true;
        }
    }
    return keyExists;
} 

std::vector<std::string> KvStore::keys() const
{
    std::vector<std::string> keys ;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (const auto& [key, value] : m_store) 
        {
             keys.push_back(key);
        }
    }
    return keys;
}
    
void KvStore::flush()
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_store.clear();
    }
   
}

  
std::unordered_map<std::string, std::string> KvStore::getSnapShot() const
{
   std::unordered_map<std::string, std::string> kvStoreSnapshot ;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        kvStoreSnapshot = m_store ;
    
    }
    return kvStoreSnapshot ;
}

void KvStore::restoreEntry(const std::string& inComingKey, const std::string& inComingValue) 
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_store[inComingKey] = inComingValue ;
    }

}
    