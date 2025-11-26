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
#include "rocstorage/interfaces.hpp"
#include <unistd.h>
#include <fstream>
#include <functional>

namespace rocstorage
{

/// Default node info provider - uses hostname and /etc/machine-id
class default_node_info : public node_info_provider
{
public:
    default_node_info()
    {
        // Try to read machine-id
        std::ifstream ifs("/etc/machine-id");
        if (ifs.is_open())
        {
            ifs >> m_node_id;
        }

        // Fallback to hostname
        if (m_node_id.empty())
        {
            char hostname[256] = {0};
            gethostname(hostname, sizeof(hostname));
            m_node_id = hostname;
        }

        m_hash = std::hash<std::string>{}(m_node_id);
    }

    std::string get_node_id() const override { return m_node_id; }
    uint64_t get_node_hash() const override { return m_hash; }

private:
    std::string m_node_id;
    uint64_t m_hash = 0;
};

}  // namespace rocstorage