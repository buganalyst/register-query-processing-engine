#include "database.hpp"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <stdexcept>

Database::Database() : storage_("data"), memory_(storage_, 3, 4) {}

bool Database::execute(const std::string& sql) {
    if (sql.empty()) return true;
    std::string command = sql;
    std::transform(command.begin(), command.end(), command.begin(), [](unsigned char c) { return std::toupper(c); });
    if (command == "STATS" || command == "SHOW STATS") { printStats(); return true; }
    Statement statement = parser_.parse(sql);
    if (statement.type == StatementType::CreateTable) {
        bool created = storage_.createTable({statement.table, statement.columns});
        std::cout << (created ? "Table created: " : "Table already exists: ") << statement.table << '\n';
    } else if (statement.type == StatementType::Insert) {
        storage_.insertRow(statement.table, statement.values);
        std::cout << "1 row inserted into " << statement.table << '\n';
    } else if (statement.type == StatementType::Select) {
        return executeSelect(statement);
    }
    return true;
}

bool Database::executeSelect(const Statement& statement) {
    auto schema = storage_.getSchema(statement.table);
    std::size_t whereIndex = 0;
    if (statement.hasWhere) {
        auto it = std::find_if(schema.columns.begin(), schema.columns.end(), [&](const Column& c) { return c.name == statement.whereColumn; });
        if (it == schema.columns.end()) throw std::runtime_error("Unknown WHERE column: " + statement.whereColumn);
        whereIndex = static_cast<std::size_t>(std::distance(schema.columns.begin(), it));
    }
    std::cout << '\n';
    for (const auto& column : schema.columns) std::cout << std::left << std::setw(18) << column.name;
    std::cout << "\n";
    for (std::size_t i = 0; i < schema.columns.size() * 18; ++i) std::cout << '-';
    std::cout << "\n";
    std::size_t returned = 0;
    const std::size_t pages = storage_.pageCount(statement.table, memory_.rowsPerPage());
    for (std::size_t page = 0; page < pages; ++page) {
        auto rows = memory_.fetchPage(statement.table, page);
        for (const auto& row : rows) {
            if (statement.hasWhere && (whereIndex >= row.size() || row[whereIndex] != statement.whereValue)) continue;
            for (const auto& value : row) std::cout << std::left << std::setw(18) << value;
            std::cout << '\n';
            ++returned;
        }
    }
    std::cout << returned << " row(s) returned\n";
    printStats();
    return true;
}

void Database::printStats() const {
    const auto& pool = memory_.bufferPool();
    const auto total = pool.hits() + pool.misses();
    const double hitRate = total == 0 ? 0.0 : 100.0 * static_cast<double>(pool.hits()) / static_cast<double>(total);
    std::cout << "\nMemory statistics\n";
    std::cout << "Frames: " << pool.size() << '/' << memory_.frameCount() << '\n';
    std::cout << "Page faults: " << memory_.pageFaults() << '\n';
    std::cout << "Disk reads: " << memory_.diskReads() << '\n';
    std::cout << "Cache hits: " << pool.hits() << '\n';
    std::cout << "Cache misses: " << pool.misses() << '\n';
    std::cout << "Cache hit rate: " << std::fixed << std::setprecision(2) << hitRate << "%\n";
    std::cout << "Page evictions: " << pool.evictions() << "\n\n";
}
