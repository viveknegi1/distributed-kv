cmake_minimum_required(VERSION 3.15)
project(distributed-kv VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  #

add_compile_options(-Wall -Wextra -Wpedantic)

add_executable(distributed-kv 
    src/kv_store.cpp
    src/main.cpp
    src/logger.cpp
    src/node_config.cpp
    src/peer_connection.cpp
    src/peer_manager.cpp
    src/rpc_server.cpp
)
target_include_directories(distributed-kv PRIVATE include)
