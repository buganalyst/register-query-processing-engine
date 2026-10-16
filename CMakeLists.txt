cmake_minimum_required(VERSION 3.16)
project(RegisterQueryEngine LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_executable(register_query_engine
    src/main.cpp
    src/sql_parser.cpp
    src/storage.cpp
    src/buffer_pool.cpp
    src/virtual_memory.cpp
    src/database.cpp
)
target_include_directories(register_query_engine PRIVATE include)
