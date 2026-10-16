#include "database.hpp"
#include <iostream>
#include <string>

int main() {
    Database database;
    std::cout << "Register Query Processing Engine with Virtual Memory and Paging\n";
    std::cout << "Supported: CREATE TABLE, INSERT INTO, SELECT * FROM, STATS, EXIT\n";
    std::cout << "Rows per page: 4 | Frames: 3 | Replacement: LRU\n\n";
    std::string line;
    while (true) {
        std::cout << "rqe> ";
        if (!std::getline(std::cin, line)) break;
        if (line == "EXIT" || line == "exit" || line == "quit") break;
        try { database.execute(line); }
        catch (const std::exception& error) { std::cout << "Error: " << error.what() << '\n'; }
    }
    std::cout << "Engine closed.\n";
    return 0;
}
