#pragma once
#include "types.hpp"
#include <string>

class SQLParser {
public:
    Statement parse(const std::string& sql) const;
};
