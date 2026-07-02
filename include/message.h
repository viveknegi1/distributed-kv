#pragma once
#include <string>
#include <variant>


   
    struct SetCommand
    {
        std::string key ;
        std::string value ;
    };

     struct DeleteCommand
    {
        std::string key ;
    };

    struct FlushCommand
    {

    };

    using Command = std::variant<SetCommand, DeleteCommand, FlushCommand>;

    struct LogEntry
    {
        Command commandType;
        int term;
        int index;

    };

