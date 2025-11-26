# rocstorage

Generic SQLite-based storage library for profiling data.

## Overview

rocstorage provides a type-safe C++ wrapper around SQLite3 for storing
structured profiling data. Originally extracted from rocprofiler-systems,
it's now a standalone, reusable library.

## Features

- Type-safe value binding using C++ templates
- Prepared statement caching for performance
- SQL schema generation from .sql files
- Support for complex queries via builder pattern
- Thread-safe execution
- Zero-copy string handling where possible

## Requirements

- CMake 3.16+
- C++17 compiler
- SQLite3 (either system or built from source)

## Building

```bash
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/path/to/install
cmake --build .
cmake --install .
```

## Usage

```cpp
#include <rocstorage/database.hpp>

using namespace rocstorage;

// Create database
database db(pid, ppid);
db.initialize_schema();

// Create prepared statement
auto insert_event = db.create_statement_executor<int64_t, const char*>(
    "INSERT INTO events (timestamp, name) VALUES (?, ?)"
);

// Execute with type-safe binding
insert_event(1234567890, "kernel_launch");
```

## Integration

### CMake find_package

```cmake
find_package(rocstorage 1.0 REQUIRED)
target_link_libraries(your-target PRIVATE rocstorage::rocstorage)
```

### CMake add_subdirectory

```cmake
add_subdirectory(path/to/rocstorage)
target_link_libraries(your-target PRIVATE rocstorage)
```

## License

MIT License - see LICENSE file

## Contributing

This project follows semantic versioning. Submit issues and PRs to the repository.
