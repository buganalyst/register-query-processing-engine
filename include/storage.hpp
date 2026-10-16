#pragma once
#include "types.hpp"
#include <filesystem>
#include <string>
#include <vector>

class Storage {
public:
    explicit Storage(std::string root = "data");
    bool createTable(const TableSchema& schema);
    bool tableExists(const std::string& table) const;
    TableSchema getSchema(const std::string& table) const;
    bool insertRow(const std::string& table, const Row& row);
    std::size_t rowCount(const std::string& table) const;
    std::size_t pageCount(const std::string& table, std::size_t rowsPerPage) const;
    std::vector<Row> readPage(const std::string& table, std::size_t page, std::size_t rowsPerPage) const;
    std::vector<Row> readAll(const std::string& table) const;
private:
    std::string root_;
    std::string schemaPath(const std::string& table) const;
    std::string dataPath(const std::string& table) const;
    static std::string clean(const std::string& value);
    static std::vector<std::string> split(const std::string& line, char delimiter);
};
