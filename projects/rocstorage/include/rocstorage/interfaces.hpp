// MIT License
//
// Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <memory>
#include <string>
#include <cstdarg>

namespace rocstorage
{

/// Logger interface - allows custom logging implementations
struct logger
{
    virtual ~logger() = default;

    /// Log a warning message (level 0 = always, higher = more verbose required)
    virtual void warning(int level, const char* fmt, ...) = 0;

    /// Log a verbose/info message
    virtual void verbose(int level, const char* fmt, ...) = 0;
};

/// Node information provider - allows custom node identification
struct node_info_provider
{
    virtual ~node_info_provider() = default;

    /// Get a unique identifier for this node/machine
    virtual std::string get_node_id() const = 0;

    /// Get a hash of the node ID (for UPID generation)
    virtual uint64_t get_node_hash() const = 0;
};

/// Configuration for database creation
struct database_config
{
    int pid = 0;
    int ppid = 0;
    std::string db_path = "";  // Empty = use default path logic

    std::shared_ptr<logger> log = nullptr;              // null = use default
    std::shared_ptr<node_info_provider> node = nullptr; // null = use default
};

}  // namespace rocstorage