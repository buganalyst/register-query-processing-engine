#include "sql_parser.hpp"
#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace {
std::string trim(std::string value) {
    auto first = std::find_if_not(value.begin(), value.end(), [](unsigned char c) { return std::isspace(c); });
    auto last = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char c) { return std::isspace(c); }).base();
    if (first >= last) return "";
    return std::string(first, last);
}
std::string upper(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return std::toupper(c); });
    return value;
}
std::vector<std::string> splitComma(const std::string& text) {
    std::vector<std::string> parts;
    std::string current;
    char quote = 0;
    for (char c : text) {
        if ((c == '\'' || c == '"') && (!quote || quote == c)) quote = quote ? 0 : c;
        if (c == ',' && !quote) { parts.push_back(trim(current)); current.clear(); }
        else current += c;
    }
    if (!trim(current).empty()) parts.push_back(trim(current));
    return parts;
}
std::string unquote(std::string value) {
    value = trim(value);
    if (value.size() >= 2 && ((value.front() == '\'' && value.back() == '\'') || (value.front() == '"' && value.back() == '"')))
        return value.substr(1, value.size() - 2);
    return value;
}
}

Statement SQLParser::parse(const std::string& input) const {
    std::string sql = trim(input);
    if (!sql.empty() && sql.back() == ';') sql.pop_back();
    std::string normalized = upper(sql);
    Statement result;

    if (normalized.rfind("CREATE TABLE ", 0) == 0) {
        auto open = sql.find('(');
        auto close = sql.rfind(')');
        if (open == std::string::npos || close == std::string::npos || close < open) throw std::runtime_error("Expected CREATE TABLE name (column TYPE, ...)");
        result.type = StatementType::CreateTable;
        result.table = trim(sql.substr(13, open - 13));
        for (const auto& part : splitComma(sql.substr(open + 1, close - open - 1))) {
            auto separator = part.find_first_of(" \t");
            if (separator == std::string::npos) throw std::runtime_error("Each column needs a name and type");
            result.columns.push_back({trim(part.substr(0, separator)), upper(trim(part.substr(separator + 1)))});
        }
        if (result.table.empty() || result.columns.empty()) throw std::runtime_error("Table name and columns are required");
        return result;
    }

    if (normalized.rfind("INSERT INTO ", 0) == 0) {
        auto valuesPos = normalized.find(" VALUES ");
        if (valuesPos == std::string::npos) throw std::runtime_error("Expected INSERT INTO table VALUES (...)");
        result.type = StatementType::Insert;
        result.table = trim(sql.substr(12, valuesPos - 12));
        auto open = sql.find('(', valuesPos);
        auto close = sql.rfind(')');
        if (open == std::string::npos || close == std::string::npos || close < open) throw std::runtime_error("Expected values inside parentheses");
        for (const auto& part : splitComma(sql.substr(open + 1, close - open - 1))) result.values.push_back(unquote(part));
        return result;
    }

    if (normalized.rfind("SELECT * FROM ", 0) == 0) {
        result.type = StatementType::Select;
        auto wherePos = normalized.find(" WHERE ");
        if (wherePos == std::string::npos) result.table = trim(sql.substr(14));
        else {
            result.table = trim(sql.substr(14, wherePos - 14));
            std::string condition = trim(sql.substr(wherePos + 7));
            auto equals = condition.find('=');
            if (equals == std::string::npos) throw std::runtime_error("WHERE currently supports column = value");
            result.hasWhere = true;
            result.whereColumn = trim(condition.substr(0, equals));
            result.whereValue = unquote(condition.substr(equals + 1));
        }
        if (result.table.empty()) throw std::runtime_error("Table name is required");
        return result;
    }

    throw std::runtime_error("Supported commands: CREATE TABLE, INSERT INTO, SELECT * FROM, STATS, EXIT");
}
