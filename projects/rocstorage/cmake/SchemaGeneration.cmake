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
# rocstorage_generate_schema_headers
#
# Generates C++ headers from SQL schema files
#
# ------------------------------------------------------------------------------#

function(rocstorage_generate_schema_headers)
    set(SCHEMA_FILES
        "rocpd_tables.sql"
        "rocpd_views.sql"
        "data_views.sql"
        "marker_views.sql"
        "summary_views.sql"
    )

    set(SCHEMA_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/schema")
    set(SCHEMA_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/include/rocstorage/schema")
    set(TEMPLATE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/schema_template.hpp.in")

    file(MAKE_DIRECTORY ${SCHEMA_BINARY_DIR})

    foreach(SCHEMA_FILE ${SCHEMA_FILES})
        file(READ "${SCHEMA_SOURCE_DIR}/${SCHEMA_FILE}" SQL_CONTENT)

        # Escape special characters for C++ string literal
        string(REPLACE "\\" "\\\\" SQL_CONTENT "${SQL_CONTENT}")
        string(REPLACE "\"" "\\\"" SQL_CONTENT "${SQL_CONTENT}")
        string(REPLACE "\n" "\\n\"\n\"" SQL_CONTENT "${SQL_CONTENT}")

        get_filename_component(SCHEMA_NAME ${SCHEMA_FILE} NAME_WE)
        string(TOUPPER ${SCHEMA_NAME} SCHEMA_NAME_UPPER)

        configure_file(
            "${TEMPLATE_FILE}"
            "${SCHEMA_BINARY_DIR}/${SCHEMA_NAME}.hpp"
            @ONLY
        )
    endforeach()
endfunction()