# MIT License
#
# Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

include_guard(GLOBAL)

# ------------------------------------------------------------------------------#
#
# SQLite3 dependency
#
# ------------------------------------------------------------------------------#

option(ROCSTORAGE_BUILD_SQLITE3 "Build SQLite3 from source" OFF)

if(ROCSTORAGE_BUILD_SQLITE3)
    message(STATUS "[${PROJECT_NAME}] Building SQLite3 from source")
    include(ExternalProject)

    ExternalProject_Add(
        sqlite3-build
        URL https://www.sqlite.org/2024/sqlite-autoconf-3450300.tar.gz
        URL_HASH SHA256=cd9c27841b7a5932c9897651e20b86c701dd740556989b01ca596fcfa3d49a0a
        PREFIX ${CMAKE_BINARY_DIR}/external/sqlite3
        CONFIGURE_COMMAND
            <SOURCE_DIR>/configure
            --prefix=<INSTALL_DIR>
            --disable-shared
            --enable-static
        BUILD_COMMAND make
        INSTALL_COMMAND make install
    )

    add_library(rocstorage-sqlite3 INTERFACE)

    target_link_libraries(
        rocstorage-sqlite3
        INTERFACE ${CMAKE_BINARY_DIR}/external/sqlite3/lib/libsqlite3.a
    )

    target_include_directories(
        rocstorage-sqlite3
        SYSTEM INTERFACE ${CMAKE_BINARY_DIR}/external/sqlite3/include
    )

    add_dependencies(rocstorage-sqlite3 sqlite3-build)
else()
    find_package(SQLite3 3.24 REQUIRED)
    add_library(rocstorage-sqlite3 ALIAS SQLite::SQLite3)
endif()