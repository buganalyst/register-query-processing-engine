#include "storage.hpp"
#include <fstream>
#include <stdexcept>

Storage::Storage(std::string root) : root_(std::move(root)) {
    std::filesystem::create_directories(root_);
}
std::string Storage::schemaPath(const std::string& table) const { return root_ + "/" + table + ".schema"; }
std::string Storage::dataPath(const std::string& table) const { return root_ + "/" + table + ".tbl"; }
std::string Storage::clean(const std::string& value) {
    if (value.find('|') != std::string::npos || value.find('\n') != std::string::npos || value.find('\r') != std::string::npos)
        throw std::runtime_error("Values cannot contain | or newline characters");
    return value;
}
std::vector<std::string> Storage::split(const std::string& line, char delimiter) {
    std::vector<std::string> parts;
    std::string item;
    for (char c : line) { if (c == delimiter) { parts.push_back(item); item.clear(); } else item += c; }
    parts.push_back(item);
    return parts;
}
bool Storage::createTable(const TableSchema& schema) {
    if (tableExists(schema.name)) return false;
    std::ofstream out(schemaPath(schema.name));
    if (!out) throw std::runtime_error("Cannot create schema file");
    for (const auto& column : schema.columns) out << clean(column.name) << '|' << clean(column.type) << '\n';
    std::ofstream data(dataPath(schema.name));
    return static_cast<bool>(data);
}
bool Storage::tableExists(const std::string& table) const { return std::filesystem::exists(schemaPath(table)); }
TableSchema Storage::getSchema(const std::string& table) const {
    std::ifstream in(schemaPath(table));
    if (!in) throw std::runtime_error("Table does not exist: " + table);
    TableSchema schema; schema.name = table;
    std::string line;
    while (std::getline(in, line)) {
        auto fields = split(line, '|');
        if (fields.size() == 2) schema.columns.push_back({fields[0], fields[1]});
    }
    return schema;
}
bool Storage::insertRow(const std::string& table, const Row& row) {
    auto schema = getSchema(table);
    if (row.size() != schema.columns.size()) throw std::runtime_error("Value count does not match table column count");
    std::ofstream out(dataPath(table), std::ios::app);
    if (!out) throw std::runtime_error("Cannot open table data file");
    for (std::size_t i = 0; i < row.size(); ++i) { if (i) out << '|'; out << clean(row[i]); }
    out << '\n';
    return static_cast<bool>(out);
}
std::vector<Row> Storage::readAll(const std::string& table) const {
    if (!tableExists(table)) throw std::runtime_error("Table does not exist: " + table);
    std::ifstream in(dataPath(table));
    std::vector<Row> rows;
    std::string line;
    while (std::getline(in, line)) if (!line.empty()) rows.push_back(split(line, '|'));
    return rows;
}
std::size_t Storage::rowCount(const std::string& table) const { return readAll(table).size(); }
std::size_t Storage::pageCount(const std::string& table, std::size_t rowsPerPage) const {
    const auto count = rowCount(table);
    return count == 0 ? 0 : (count + rowsPerPage - 1) / rowsPerPage;
}
std::vector<Row> Storage::readPage(const std::string& table, std::size_t page, std::size_t rowsPerPage) const {
    auto rows = readAll(table);
    const std::size_t begin = page * rowsPerPage;
    if (begin >= rows.size()) return {};
    const std::size_t end = std::min(rows.size(), begin + rowsPerPage);
    return std::vector<Row>(rows.begin() + begin, rows.begin() + end);
}
