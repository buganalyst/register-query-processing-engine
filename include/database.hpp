#pragma once
#include "sql_parser.hpp"
#include "virtual_memory.hpp"
#include <string>

class Database {
public:
    Database();
    bool execute(const std::string& sql);
private:
    Storage storage_;
    VirtualMemoryManager memory_;
    SQLParser parser_;
    void printStats() const;
    bool executeSelect(const Statement& statement);
};
