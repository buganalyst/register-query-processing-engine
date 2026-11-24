#include "database.hpp"
#include <filesystem>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc > 1 && std::string(argv[1]) == "--gui") {
        std::filesystem::path executable = argv[0];
        std::filesystem::path gui = executable.parent_path() / "gui.py";
        std::string python = "python3";
#ifdef _WIN32
        python = "python";
#endif
        std::string command = python + " \"" + gui.string() + "\" --engine \"" +
                              std::filesystem::absolute(executable).string() + "\"";
        return std::system(command.c_str());
    }

    Database database;
    std::cout << "Register Query Processing Engine with Virtual Memory and Paging\n";
    std::cout << "Supported: CREATE TABLE, INSERT INTO, SELECT * FROM, STATS, EXIT\n";
    std::cout << "Rows per page: 4 | Frames: 3 | Replacement: LRU\n\n";
    std::string line;
    while (true) {
        std::cout << "rqe> " << std::flush;
        if (!std::getline(std::cin, line)) break;
        if (line == "EXIT" || line == "exit" || line == "quit") break;
        try { database.execute(line); }
        catch (const std::exception& error) { std::cout << "Error: " << error.what() << '\n'; }
    }
    std::cout << "Engine closed.\n";
    return 0;
}
