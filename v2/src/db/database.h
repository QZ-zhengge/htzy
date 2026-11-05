// Database - MySQL C API wrapper (mysql.h, not JDBC)
// More stable on Windows, no JDBC driver loading issues
#pragma once

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <mysql.h>
#include "../common/logger.h"

using Row = std::map<std::string, std::string>;
using Rows = std::vector<Row>;

// SQL-safe string escape (for use in string-concatenated SQL)
static std::string sqlEscape(const std::string& input) {
    std::string out;
    out.reserve(input.size() + input.size() / 4);
    for (char c : input) {
        if (c == '\\') out += "\\\\";
        else if (c == '\'') out += "\\'";
        else if (c == '"') out += "\\\"";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "\\r";
        else if (c == '\0') out += "\\0";
        else out += c;
    }
    if (out.size() > 50000) out = out.substr(0, 50000);
    return out;
}

class Database {
public:
    static Database& instance() {
        static Database inst;
        return inst;
    }

    bool connect(const std::string& host, int port,
                 const std::string& user, const std::string& password,
                 const std::string& dbname) {
        std::lock_guard<std::mutex> lock(mu_);
        host_ = host; port_ = port; user_ = user; password_ = password; dbname_ = dbname;
        if (!doConnect(host, port, user, password)) return false;
        if (mysql_select_db(conn_, dbname.c_str()) != 0) {
            LOG_ERROR("DB", std::string("select_db failed: ") + mysql_error(conn_));
            return false;
        }
        LOG_INFO("DB", "Connected: " + host + ":" + std::to_string(port) + "/" + dbname);
        return true;
    }

    bool connectWithoutDb(const std::string& host, int port,
                          const std::string& user, const std::string& password) {
        std::lock_guard<std::mutex> lock(mu_);
        return doConnect(host, port, user, password);
    }

    void disconnect() {
        std::lock_guard<std::mutex> lock(mu_);
        if (conn_) { mysql_close(conn_); conn_ = nullptr; }
    }

    bool execute(const std::string& sql) {
        std::lock_guard<std::mutex> lock(mu_);
        if (!conn_) { reconnect(); }
        if (!conn_) return false;
        if (mysql_query(conn_, sql.c_str()) != 0) {
            std::string err = mysql_error(conn_);
            // If lost connection, try reconnect once
            if (err.find("disconnected") != std::string::npos || err.find("Lost connection") != std::string::npos) {
                LOG_WARN("DB", "Connection lost, reconnecting...");
                reconnect();
                if (conn_ && mysql_query(conn_, sql.c_str()) == 0) return true;
            }
            LOG_ERROR("DB", std::string("SQL failed: ") + err);
            return false;
        }
        return true;
    }

    Rows query(const std::string& sql) {
        std::lock_guard<std::mutex> lock(mu_);
        Rows result;
        if (!conn_) { reconnect(); }
        if (!conn_) return result;
        if (mysql_query(conn_, sql.c_str()) != 0) {
            std::string err = mysql_error(conn_);
            if (err.find("disconnected") != std::string::npos || err.find("Lost connection") != std::string::npos) {
                LOG_WARN("DB", "Connection lost, reconnecting...");
                reconnect();
                if (conn_) {
                    if (mysql_query(conn_, sql.c_str()) != 0) {
                        LOG_ERROR("DB", std::string("Query failed after reconnect: ") + mysql_error(conn_));
                        return result;
                    }
                } else {
                    return result;
                }
            } else {
                LOG_ERROR("DB", std::string("Query failed: ") + err);
                return result;
            }
        }
        MYSQL_RES* res = mysql_store_result(conn_);
        if (!res) {
            LOG_ERROR("DB", std::string("store_result failed: ") + mysql_error(conn_));
            return result;
        }
        int num_fields = mysql_num_fields(res);
        MYSQL_FIELD* fields = mysql_fetch_fields(res);
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res))) {
            Row r;
            for (int i = 0; i < num_fields; ++i) {
                r[fields[i].name] = row[i] ? row[i] : "";
            }
            result.push_back(r);
        }
        mysql_free_result(res);
        return result;
    }

    bool executePrepared(const std::string& sql, std::vector<std::string> params = {}) {
        std::lock_guard<std::mutex> lock(mu_);
        if (!conn_) return false;
        MYSQL_STMT* stmt = mysql_stmt_init(conn_);
        if (!stmt) {
            LOG_ERROR("DB", "stmt_init failed");
            return false;
        }
        if (mysql_stmt_prepare(stmt, sql.c_str(), static_cast<unsigned long>(sql.length())) != 0) {
            LOG_ERROR("DB", std::string("stmt_prepare failed: ") + mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return false;
        }
        if (!params.empty()) {
            std::vector<MYSQL_BIND> binds(params.size());
            std::vector<unsigned long> lengths(params.size());
            std::vector<unsigned char> null_bits(params.size(), 0);
            memset(&binds[0], 0, sizeof(MYSQL_BIND) * params.size());
            for (size_t i = 0; i < params.size(); ++i) {
                binds[i].buffer_type = MYSQL_TYPE_STRING;
                binds[i].buffer = const_cast<char*>(params[i].c_str());
                binds[i].buffer_length = static_cast<unsigned long>(params[i].size());
                binds[i].is_null = reinterpret_cast<bool*>(&null_bits[i]);
                binds[i].length = &lengths[i];
                lengths[i] = static_cast<unsigned long>(params[i].size());
            }
            mysql_stmt_bind_param(stmt, &binds[0]);
        }
        if (mysql_stmt_execute(stmt) != 0) {
            LOG_ERROR("DB", std::string("stmt_execute failed: ") + mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return false;
        }
        mysql_stmt_close(stmt);
        return true;
    }

    uint64_t lastInsertId() {
        std::lock_guard<std::mutex> lock(mu_);
        if (!conn_) return 0;
        return mysql_insert_id(conn_);
    }

    bool createDatabase(const std::string& dbname) {
        return execute("CREATE DATABASE IF NOT EXISTS " + dbname +
                       " CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci");
    }

    bool useDatabase(const std::string& dbname) {
        std::lock_guard<std::mutex> lock(mu_);
        if (!conn_) return false;
        return mysql_select_db(conn_, dbname.c_str()) == 0;
    }

    bool isConnected() const {
        std::lock_guard<std::mutex> lock(mu_);
        return conn_ != nullptr;
    }

private:
    Database() = default;
    ~Database() { disconnect(); }

    MYSQL* conn_ = nullptr;
    mutable std::mutex mu_;
    std::string host_, user_, password_, dbname_;
    int port_ = 3306;

    bool reconnect() {
        LOG_INFO("DB", "Attempting reconnect...");
        if (conn_) { mysql_close(conn_); conn_ = nullptr; }
        if (!doConnect(host_, port_, user_, password_)) return false;
        if (!dbname_.empty()) mysql_select_db(conn_, dbname_.c_str());
        LOG_INFO("DB", "Reconnected successfully");
        return true;
    }

    bool doConnect(const std::string& host, int port,
                   const std::string& user, const std::string& password) {
        conn_ = mysql_init(nullptr);
        if (!conn_) {
            LOG_ERROR("DB", "mysql_init() failed");
            return false;
        }

        // Set connection timeout
        unsigned int timeout = 10;
        mysql_options(conn_, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);

        LOG_INFO("DB", "Connecting to " + host + ":" + std::to_string(port) + " as " + user + "...");

        if (!mysql_real_connect(conn_, host.c_str(), user.c_str(), password.c_str(),
                                nullptr, port, nullptr, 0)) {
            LOG_ERROR("DB", std::string("Connect failed: ") + mysql_error(conn_));
            LOG_ERROR("DB", "Check: 1) MySQL service running  2) root password correct");
            mysql_close(conn_);
            conn_ = nullptr;
            return false;
        }

        LOG_INFO("DB", "MySQL connected successfully");
        return true;
    }
};
