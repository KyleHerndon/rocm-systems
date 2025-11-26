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

#include <rocstorage/database.hpp>
#include <rocstorage/interfaces.hpp>
#include "config.hpp"
#include "debug.hpp"
#include "node_info.hpp"

#include <memory>
#include <cstdarg>

namespace rocprofsys
{

namespace rocpd_integration
{

/// Logger implementation using rocprofiler-systems debug infrastructure
class rocpd_logger : public ::rocstorage::logger
{
public:
    void warning(int level, const char* fmt, ...) override
    {
        if (config::get_debug() || config::get_verbose() >= level)
        {
            char buffer[1024];
            va_list args;
            va_start(args, fmt);
            vsnprintf(buffer, sizeof(buffer), fmt, args);
            va_end(args);
            ROCPROFSYS_WARNING(level, "%s", buffer);
        }
    }

    void verbose(int level, const char* fmt, ...) override
    {
        if (config::get_debug() || config::get_verbose() >= level)
        {
            char buffer[1024];
            va_list args;
            va_start(args, fmt);
            vsnprintf(buffer, sizeof(buffer), fmt, args);
            va_end(args);
            ROCPROFSYS_VERBOSE(level, "%s", buffer);
        }
    }
};

/// Node info provider using rocprofiler-systems node_info
class rocpd_node_info : public ::rocstorage::node_info_provider
{
public:
    std::string get_node_id() const override
    {
        return node_info::get_instance().machine_id;
    }

    uint64_t get_node_hash() const override
    {
        return node_info::get_instance().hash;
    }
};

}  // namespace rocpd_integration

/// Factory function to create a rocpd database configured for rocprofiler-systems
inline std::shared_ptr<::rocstorage::database>
create_rocpd_database(int pid, int ppid)
{
    ::rocstorage::database_config cfg;
    cfg.pid = pid;
    cfg.ppid = ppid;

    // Use rocprofiler-systems path resolution
    cfg.db_path = config::get_database_absolute_path("rocpd", std::to_string(pid));

    // Use rocprofiler-systems logging
    cfg.log = std::make_shared<rocpd_integration::rocpd_logger>();

    // Use rocprofiler-systems node info
    cfg.node = std::make_shared<rocpd_integration::rocpd_node_info>();

    return std::make_shared<::rocstorage::database>(cfg);
}

}  // namespace rocprofsys
