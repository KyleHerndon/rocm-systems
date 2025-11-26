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

#include "rocstorage/database.hpp"
#include "rocstorage/md5sum.hpp"
#include "default_logger.hpp"
#include "default_node_info.hpp"

#include <regex>
#include <string>
#include <cstdlib>
#include <sys/stat.h>
#include <unistd.h>

#if defined(ROCPROFSYS_USE_ROCPD_LIBRARY) && ROCPROFSYS_USE_ROCPD_LIBRARY > 0
#    include <rocprofiler-sdk-rocpd/rocpd.h>
#    include <rocprofiler-sdk-rocpd/types.h>
#else
#    include "rocstorage/schema/data_views.hpp"
#    include "rocstorage/schema/marker_views.hpp"
#    include "rocstorage/schema/rocpd_tables.hpp"
#    include "rocstorage/schema/rocpd_views.hpp"
#    include "rocstorage/schema/summary_views.hpp"

namespace
{
enum rocpd_sql_schema_kind_t
{
    ROCPD_SQL_SCHEMA_NONE = 0,
    ROCPD_SQL_SCHEMA_ROCPD_TABLES,
    ROCPD_SQL_SCHEMA_ROCPD_INDEXES,
    ROCPD_SQL_SCHEMA_ROCPD_VIEWS,
    ROCPD_SQL_SCHEMA_ROCPD_DATA_VIEWS,
    ROCPD_SQL_SCHEMA_ROCPD_SUMMARY_VIEWS,
    ROCPD_SQL_SCHEMA_ROCPD_MARKER_VIEWS,
    ROCPD_SQL_SCHEMA_LAST,
};
}  // namespace

#endif

namespace
{
std::string
get_dirname(const std::string& path)
{
    size_t pos = path.find_last_of("/\\");
    return (pos != std::string::npos) ? path.substr(0, pos) : ".";
}

bool
dir_exists(const std::string& path)
{
    struct stat info;
    return stat(path.c_str(), &info) == 0 && (info.st_mode & S_IFDIR);
}

void
make_dir(const std::string& path)
{
    if (!dir_exists(path))
    {
        // Try to create parent directory first if needed
        std::string parent = get_dirname(path);
        if (!parent.empty() && parent != "." && parent != "/" && !dir_exists(parent))
        {
            make_dir(parent);
        }
        mkdir(path.c_str(), 0755);
    }
}

std::string
get_default_db_path(int pid)
{
    const char* dir = std::getenv("ROCSTORAGE_DATABASE_DIR");
    if (!dir) dir = std::getenv("ROCPROFSYS_DATABASE_DIR");

    std::string base = dir ? std::string(dir) : ".";
    return base + "/rocpd-" + std::to_string(pid) + ".db";
}

std::string
make_absolute(const std::string& path)
{
    if (!path.empty() && path[0] != '/')
    {
        const char* pwd = std::getenv("PWD");
        if (pwd) return std::string(pwd) + "/" + path;
    }
    return path;
}

std::string
process_schema_template(std::string_view schema_content, const std::string& upid)
{
    std::string query = std::string(schema_content);

    std::regex upid_pattern("\\{\\{uuid\\}\\}");
    std::regex guid_pattern("\\{\\{guid\\}\\}");
    std::regex view_upid_pattern("\\{\\{view_upid\\}\\}");

    query = std::regex_replace(query, upid_pattern, "_" + upid);
    query = std::regex_replace(query, guid_pattern, upid);
    query = std::regex_replace(query, view_upid_pattern, "");

    return query;
}

#if defined(ROCPROFSYS_USE_ROCPD_LIBRARY) && ROCPROFSYS_USE_ROCPD_LIBRARY > 0
void
load_schema_cb(rocpd_sql_engine_t, rocpd_sql_schema_kind_t, rocpd_sql_options_t,
               const rocpd_sql_schema_jinja_variables_t*, const char*,
               const char* schema_content, void* user_data)
{
    if(user_data == nullptr || schema_content == nullptr)
    {
        return;
    }
    auto* query = static_cast<std::string*>(user_data);
    if(query == nullptr)
    {
        return;
    }
    *query = std::string(schema_content);
}
#endif

std::string
get_schema_query(rocpd_sql_schema_kind_t schema_kind, const std::string& upid,
                 rocstorage::logger* log)
{
#if defined(ROCPROFSYS_USE_ROCPD_LIBRARY) && ROCPROFSYS_USE_ROCPD_LIBRARY > 0
    const auto                         jinja_size = 2 * upid.size();
    rocpd_sql_schema_jinja_variables_t info{ jinja_size, upid.c_str(), upid.c_str() };

    std::string query;
    auto        status = rocpd_sql_load_schema(ROCPD_SQL_ENGINE_SQLITE3, schema_kind,
                                               ROCPD_SQL_OPTIONS_NONE, &info, load_schema_cb,
                                               nullptr, 0, &query);
    if(status != ROCPD_STATUS_SUCCESS && log)
    {
        log->warning(0, "Unable to load rocpd schema. Error code: %d", status);
    }
    return query;
#else
    std::string_view schema_content;

    switch(schema_kind)
    {
        case ROCPD_SQL_SCHEMA_ROCPD_TABLES:
            schema_content = rocstorage::schema::ROCPD_TABLES_SQL;
            break;
        case ROCPD_SQL_SCHEMA_ROCPD_VIEWS:
            schema_content = rocstorage::schema::ROCPD_VIEWS_SQL;
            break;
        case ROCPD_SQL_SCHEMA_ROCPD_DATA_VIEWS:
            schema_content = rocstorage::schema::DATA_VIEWS_SQL;
            break;
        case ROCPD_SQL_SCHEMA_ROCPD_MARKER_VIEWS:
            schema_content = rocstorage::schema::MARKER_VIEWS_SQL;
            break;
        case ROCPD_SQL_SCHEMA_ROCPD_SUMMARY_VIEWS:
            schema_content = rocstorage::schema::SUMMARY_VIEWS_SQL;
            break;
        default:
            if (log) log->warning(0, "Unknown schema kind: %d", schema_kind);
            return "";
    }

    return process_schema_template(schema_content, upid);
#endif
}

}  // namespace

namespace rocstorage
{
// Full configuration constructor
database::database(const database_config& config)
    : m_logger(config.log)
    , m_node_info(config.node)
{
    // Use defaults if not provided
    if (!m_logger)
    {
        m_logger = std::make_shared<default_logger>();
    }
    if (!m_node_info)
    {
        m_node_info = std::make_shared<default_node_info>();
    }

    // Determine database path
    std::string db_path = config.db_path;
    if (db_path.empty())
    {
        db_path = get_default_db_path(config.pid);
    }
    db_path = make_absolute(db_path);

    // Create directory if needed
    make_dir(get_dirname(db_path));

    m_logger->verbose(0, "Database: %s\r\n", db_path.c_str());

    // Open databases
    if (sqlite3_open(":memory:", &_sqlite3_db_temp) != SQLITE_OK)
    {
        m_logger->warning(0, "Failed to open in-memory database");
    }
    if (sqlite3_open(db_path.c_str(), &_sqlite3_db) != SQLITE_OK)
    {
        m_logger->warning(0, "Failed to open database: %s", db_path.c_str());
    }

    // Generate UPID
    m_upid = generate_upid(config.pid, config.ppid);
}

// Convenience constructors
database::database(int pid, int ppid)
    : database(database_config{pid, ppid, "", nullptr, nullptr})
{}

database::database(int pid, int ppid, std::string db_path)
    : database(database_config{pid, ppid, std::move(db_path), nullptr, nullptr})
{}

database::~database()
{
    sqlite3_close(_sqlite3_db_temp);
    sqlite3_close(_sqlite3_db);
}

void
database::initialize_schema()
{
    const auto upid = get_upid();

    const std::vector<rocpd_sql_schema_kind_t> schema_kinds = {
        ROCPD_SQL_SCHEMA_ROCPD_TABLES, ROCPD_SQL_SCHEMA_ROCPD_VIEWS,
        ROCPD_SQL_SCHEMA_ROCPD_DATA_VIEWS, ROCPD_SQL_SCHEMA_ROCPD_MARKER_VIEWS,
        ROCPD_SQL_SCHEMA_ROCPD_SUMMARY_VIEWS
    };

    for(const auto& schema_kind : schema_kinds)
    {
        const std::string query = get_schema_query(schema_kind, upid, m_logger.get());

        if(query.empty())
        {
            if (m_logger)
            {
                m_logger->warning(0, "Failed to get schema query for schema kind: %d",
                                  schema_kind);
            }
            continue;
        }

        validate_sqlite3_result(sqlite3_exec(_sqlite3_db_temp, query.c_str(), 0, 0, 0),
                                query.c_str(),
                                std::string("Invalid schema, init database failed!"));
    }
}

void
database::execute_query(const std::string& query)
{
    validate_sqlite3_result(sqlite3_exec(_sqlite3_db_temp, query.c_str(), 0, 0, 0),
                            "Failed to execute query - ", query);
}

std::string
database::get_upid()
{
    return m_upid;
}

std::string
database::generate_upid(int pid, int ppid)
{
    auto node_id = m_node_info->get_node_id();
    auto guid = rocstorage::md5sum{ node_id, pid, ppid };
    return guid.hexdigest();
}

size_t
database::get_last_insert_id() const
{
    return sqlite3_last_insert_rowid(_sqlite3_db_temp);
}

void
database::flush()
{
    auto* backup = sqlite3_backup_init(_sqlite3_db, "main", _sqlite3_db_temp, "main");
    if(backup)
    {
        sqlite3_backup_step(backup, -1);  // Copy all pages
        sqlite3_backup_finish(backup);
    }
}

}  // namespace rocstorage