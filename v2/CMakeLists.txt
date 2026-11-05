cmake_minimum_required(VERSION 3.28)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project(HTZY_v2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- MySQL C API (auto-detect or use MYSQL_DIR env var) ----
# On Windows: searches common install paths
# On Linux: uses find_package or MYSQL_DIR environment variable
if(NOT DEFINED MYSQL_SERVER_DIR)
    if(DEFINED ENV{MYSQL_DIR})
        set(MYSQL_SERVER_DIR "$ENV{MYSQL_DIR}")
    elseif(EXISTS "C:/Program Files/MySQL/MySQL Server 9.7")
        set(MYSQL_SERVER_DIR "C:/Program Files/MySQL/MySQL Server 9.7")
    elseif(EXISTS "C:/Program Files/MySQL/MySQL Server 9.0")
        set(MYSQL_SERVER_DIR "C:/Program Files/MySQL/MySQL Server 9.0")
    elseif(EXISTS "C:/Program Files/MySQL/MySQL Server 8.0")
        set(MYSQL_SERVER_DIR "C:/Program Files/MySQL/MySQL Server 8.0")
    elseif(UNIX)
        # Linux: try common paths
        find_path(MYSQL_INCLUDE_DIR mysql.h
            PATHS /usr/include/mysql /usr/local/include/mysql
                  /opt/mysql/include)
        find_library(MYSQL_LIBRARY mysqlclient
            PATHS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu)
        if(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARY)
            set(MYSQL_SERVER_DIR_FOUND TRUE)
            set(MYSQL_INCLUDE "${MYSQL_INCLUDE_DIR}")
            set(MYSQL_LIB_DIR "${CMAKE_LIBRARY_PATH}")
            set(MYSQL_LIB_FILE "${MYSQL_LIBRARY}")
        endif()
    endif()
endif()

if(NOT MYSQL_SERVER_DIR_FOUND)
    set(MYSQL_INCLUDE "${MYSQL_SERVER_DIR}/include")
    set(MYSQL_LIB_DIR "${MYSQL_SERVER_DIR}/lib")

    if(EXISTS "${MYSQL_INCLUDE}/mysql.h" AND EXISTS "${MYSQL_LIB_DIR}/libmysql.lib")
        message(STATUS "MySQL C API include: ${MYSQL_INCLUDE}")
        message(STATUS "MySQL C API lib:     ${MYSQL_LIB_DIR}")
    else()
        message(FATAL_ERROR
            "MySQL Server not found. Set MYSQL_DIR environment variable or install MySQL Server.\n"
            "  Windows: C:/Program Files/MySQL/MySQL Server 8.0+\n"
            "  Linux: sudo apt install libmysqlclient-dev\n"
            "  Or: cmake -DMYSQL_SERVER_DIR=/path/to/mysql ..")
    endif()
endif()

# Find libmysql.dll / .so for post-build copy
if(UNIX)
    file(GLOB RUNTIME_DLLS "${CMAKE_LIBRARY_PATH}/libmysql*.so*")
else()
    file(GLOB RUNTIME_DLLS "${MYSQL_LIB_DIR}/libmysql*.dll")
endif()

# Also copy OpenSSL DLLs from MySQL bin directory (libmysql.dll depends on them)
if(NOT MYSQL_SERVER_DIR_FOUND AND EXISTS "${MYSQL_SERVER_DIR}/bin")
    file(GLOB OPENSSL_DLLS "${MYSQL_SERVER_DIR}/bin/libssl*.dll" "${MYSQL_SERVER_DIR}/bin/libcrypto*.dll")
    list(APPEND RUNTIME_DLLS ${OPENSSL_DLLS})
endif()

# ---- Source files ----
# Force UTF-8 source encoding on MSVC
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/utf-8>)

add_executable(HTZY_v2.0
    HTZY_v2.0.cpp
    src/common/config.h
    src/common/logger.h
    src/common/jwt.h
    src/db/database.h
    src/db/schema_migrator.h
    src/server/http_server.h
    src/modules/auth/auth_handler.h
    src/modules/user/user_handler.h
    src/modules/resume/resume_handler.h
    src/modules/jobs/jobs_handler.h
    src/modules/interview/interview_handler.h
    src/modules/analysis/analysis_handler.h
    src/modules/redstory/redstory_handler.h
    src/modules/admin/admin_handler.h
    src/modules/delivery/delivery_handler.h
    src/ai/deepseek_client.h
)

# ---- Include paths ----
target_include_directories(HTZY_v2.0 PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/third_party
    ${MYSQL_INCLUDE}
)

# ---- Link MySQL C API ----
if(UNIX AND MYSQL_LIB_FILE)
    target_link_libraries(HTZY_v2.0 PRIVATE ${MYSQL_LIB_FILE})
else()
    target_link_libraries(HTZY_v2.0 PRIVATE "${MYSQL_LIB_DIR}/libmysql.lib")
endif()

# ---- Post-Build: Copy MySQL runtime DLL/SO ----
foreach(D ${RUNTIME_DLLS})
    add_custom_command(TARGET HTZY_v2.0 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${D}" $<TARGET_FILE_DIR:HTZY_v2.0>
        COMMENT "Copying: ${D}"
    )
endforeach()

# ---- Post-Build: Copy config template ----
add_custom_command(TARGET HTZY_v2.0 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:HTZY_v2.0>/config"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/config/app.conf.example"
        "$<TARGET_FILE_DIR:HTZY_v2.0>/config/app.conf.example"
    COMMENT "Copying config template"
)

# ---- Post-Build: Copy frontend/dist to output ----
if(EXISTS "${CMAKE_SOURCE_DIR}/frontend/dist")
    add_custom_command(TARGET HTZY_v2.0 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:HTZY_v2.0>/frontend/dist"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/frontend/dist"
            "$<TARGET_FILE_DIR:HTZY_v2.0>/frontend/dist"
        COMMENT "Copying frontend/dist"
    )
endif()

message(STATUS "HTZY_v2.0 configured: ${CMAKE_BUILD_TYPE}")

# ---- VS Debug: do NOT set VS_DEBUGGER_WORKING_DIRECTORY ----
# VS 18 cannot parse generator expressions like $<TARGET_FILE_DIR:> or $<TARGET_PROPERTY:...>
# Instead, the program uses GetModuleFileNameA + SetCurrentDirectoryA to auto-detect exe dir.
# This is 100% reliable regardless of how VS launches the process.

# ---- Auto-copy OpenSSL DLLs from MySQL bin (needed by libmysql) ----
if(DEFINED MYSQL_SERVER_DIR)
    foreach(_dll_name libssl-3-x64.dll libcrypto-3-x64.dll)
        set(_dll_path "${MYSQL_SERVER_DIR}/bin/${_dll_name}")
        if(EXISTS "${_dll_path}")
            add_custom_command(TARGET HTZY_v2.0 POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_dll_path}" $<TARGET_FILE_DIR:HTZY_v2.0>
                COMMENT "Copying ${_dll_name}"
            )
        endif()
    endforeach()
endif()
