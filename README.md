# Register Query Processing Engine with Virtual Memory and Paging

## Overview

The **Register Query Processing Engine with Virtual Memory and Paging** is an educational project developed in C++17 to demonstrate how database query processing can interact with operating system memory-management concepts.

The project aims to build a simplified database engine that accepts a supported subset of SQL, stores records in files, and manages database pages through a simulated virtual-memory subsystem and buffer pool.

The implementation is intended for learning and demonstration. It is not designed to replace a production database management system.

## Objectives

- Implement a basic SQL parser for a defined subset of SQL.
- Store table data using file-based storage.
- Organize data access around database pages.
- Simulate page tables, memory frames, and page-fault handling.
- Implement a buffer pool with an LRU page-replacement policy.
- Collect basic memory and cache statistics.
- Provide a modular foundation for future bytecode generation and register-based virtual-machine execution.

## Current Features

The current prototype provides the following functionality, subject to the supported query syntax:

- Basic SQL commands for creating tables, inserting records, and selecting records.
- Simple equality filtering with a `WHERE` clause.
- File-based persistence for table data.
- Page-based data access and simulated virtual-memory management.
- A limited pool of memory frames.
- Buffer caching with LRU replacement.
- Statistics for cache hits, cache misses, page faults, and simulated page-read activity.

The storage and memory-management components are educational simulations. Statistics should not be interpreted as measurements of physical operating-system page faults or actual hardware disk I/O.

## Technology Stack

- **Language:** C++17
- **Compiler:** GCC with C++17 support
- **Build environment:** Windows or another environment with a compatible C++ toolchain
- **Storage:** File-based storage
- **Core concepts:** SQL parsing, database pages, virtual-memory simulation, page tables, frame allocation, buffer caching, and LRU replacement

## Project Structure

```text
register_query_engine/
├── include/
│   ├── types.hpp
│   ├── sql_parser.hpp
│   ├── storage.hpp
│   ├── buffer_pool.hpp
│   ├── virtual_memory.hpp
│   └── database.hpp
├── src/
│   ├── main.cpp
│   ├── sql_parser.cpp
│   ├── storage.cpp
│   ├── buffer_pool.cpp
│   ├── virtual_memory.cpp
│   └── database.cpp
├── data/                  # Runtime data, created as needed
├── CMakeLists.txt
└── README.md
```

### Module Responsibilities

| Module | Responsibility |
|---|---|
| `sql_parser` | Parses supported SQL statements and extracts query information. |
| `storage` | Handles file-based table storage and page-oriented data access. |
| `buffer_pool` | Caches database pages and applies the configured replacement policy. |
| `virtual_memory` | Simulates virtual-page mapping, frame allocation, and page-fault handling. |
| `database` | Coordinates query operations with storage and memory components. |
| `main` | Provides the program entry point and interactive command interface. |
| `types` | Defines shared data structures and types used by multiple modules. |

## Requirements

- A C++ compiler with C++17 support.
- Windows PowerShell, Command Prompt, or a compatible terminal.
- Git is recommended for version control.

On Windows, GCC can be installed using [MSYS2](https://www.msys2.org/). Ensure that the compiler selected in your terminal supports C++17.

## Build and Run

### Option 1: Compile with GCC on Windows

Open PowerShell in the project root directory, where the `include` and `src` folders are located.

Compile the source files:

```powershell
g++ -std=c++17 -Wall -Wextra -Iinclude src\main.cpp src\sql_parser.cpp src\storage.cpp src\buffer_pool.cpp src\virtual_memory.cpp src\database.cpp -o register_query_engine.exe
```

Run the application:

```powershell
.\register_query_engine.exe
```

If the compiler reports that the `<optional>` header is missing, check that the selected compiler supports C++17. An outdated MinGW installation may not provide the required standard-library support.

### Option 2: Build with CMake

If CMake is installed and the provided `CMakeLists.txt` is configured for your environment, run:

```powershell
cmake -S . -B build
cmake --build build
```

Run the executable from the build output directory appropriate to your generator and configuration.

## Example Usage

The following example illustrates the intended interactive workflow. Exact accepted syntax depends on the parser implementation.

```sql
CREATE TABLE students (id INT, name TEXT)
INSERT INTO students VALUES (1, 'Arin')
INSERT INTO students VALUES (2, 'Preeti')
INSERT INTO students VALUES (3, 'Anushka')
SELECT * FROM students
SELECT * FROM students WHERE id = 3
STATS
```

These commands demonstrate table creation, record insertion, retrieval, simple filtering, and statistics reporting.

## How It Works

At a high level, the prototype follows this workflow:

1. The user enters a supported SQL statement.
2. The SQL parser identifies the operation and extracts its components.
3. The database layer coordinates the requested operation.
4. The storage and memory-management modules access the required database data.
5. Database pages are mapped to simulated memory frames and may be retained in the buffer pool.
6. The engine returns the query result or reports relevant statistics.

The current prototype does not yet implement the complete compiler and virtual-machine pipeline described in the long-term project design.

## OS and DBMS Concepts Demonstrated

### Database Management Systems

- SQL statement parsing
- Table and record handling
- File-based data persistence
- Page-oriented data access
- Query filtering

### Operating Systems

- Virtual-memory simulation
- Page tables and page-to-frame mapping
- Frame allocation
- Page-fault handling
- Buffer caching and LRU replacement

## Limitations

- Only a limited subset of SQL is supported.
- The project is an educational prototype, not a production DBMS.
- The virtual-memory subsystem simulates memory-management behavior at the application level.
- The statistics represent the program's own simulation and are not equivalent to operating-system or hardware performance counters.
- Advanced SQL features, transaction management, concurrency control, and production-grade recovery are outside the current scope unless implemented in a later version.
- Bytecode generation and register-based virtual-machine execution remain future work until implemented and integrated.

## Planned Improvements

- Add SQL-to-bytecode generation.
- Implement a register-based virtual machine and instruction interpreter.
- Connect virtual-machine data requests to the virtual-memory and buffer-pool modules.
- Improve fixed-size database-page storage.
- Expand parser validation and supported SQL operations.
- Add automated tests for query handling, page mapping, cache hits and misses, and LRU replacement.
- Improve error handling, metrics, and technical documentation.

## Academic Context

This project is developed as part of a **Project Based Learning (PBL)** course for a B.Tech Computer Science and Engineering program. It combines concepts from Operating Systems and Database Management Systems in a modular C++ implementation.

## Contributing

Contributions and suggestions are welcome. For substantial changes, consider opening an issue first to discuss the proposed improvement. Keep modules focused, preserve the C++17 requirement, and include tests or reproducible examples where possible.

