#pragma once
#include <string>
#include <vector>
#include <optional>
#include <unordered_map>

struct Column {
    std::string name;
    std::string type;
};

using Row = std::vector<std::string>;

struct TableSchema {
    std::string name;
    std::vector<Column> columns;
};

enum class StatementType { CreateTable, Insert, Select, Unknown };

struct Statement {
    StatementType type = StatementType::Unknown;
    std::string table;
    std::vector<Column> columns;
    Row values;
    bool hasWhere = false;
    std::string whereColumn;
    std::string whereValue;
};

struct PageKey {
    std::string table;
    std::size_t page = 0;
    bool operator==(const PageKey& other) const {
        return table == other.table && page == other.page;
    }
};

struct PageKeyHash {
    std::size_t operator()(const PageKey& key) const {
        return std::hash<std::string>{}(key.table) ^ (std::hash<std::size_t>{}(key.page) << 1);
    }
};
