#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <string>
#include <ctime>
#include <regex>
#include <sstream>
#include <iomanip>
#include <random>
#include <cstdint>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/driver.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/resultset.h>
#include "json.hpp"

using json = nlohmann::json;

class MySQLHelper {
private:
    sql::Driver* driver;
    sql::Connection* conn;
    std::string host, user, password, database;
    unsigned int port;

    void initConnection() {
        driver = get_driver_instance();
        std::string conn_str = "tcp://" + host + ":" + std::to_string(port);
        conn = driver->connect(conn_str, user, password);
        conn->setSchema(database);
    }

public:
    MySQLHelper(const std::string& _host, unsigned int _port,
        const std::string& _user, const std::string& _pass,
        const std::string& _db)
        : host(_host), port(_port), user(_user), password(_pass), database(_db),
        driver(nullptr), conn(nullptr) {
        initConnection();
    }

    ~MySQLHelper() {
        if (conn) delete conn;
    }

    MySQLHelper(const MySQLHelper&) = delete;
    MySQLHelper& operator=(const MySQLHelper&) = delete;

    json executeQuery(const std::string& sql) {
        json result = json::array();
        try {
            sql::Statement* stmt = conn->createStatement();
            sql::ResultSet* res = stmt->executeQuery(sql);
            sql::ResultSetMetaData* meta = res->getMetaData();
            int cols = meta->getColumnCount();

            while (res->next()) {
                json row;
                for (int i = 1; i <= cols; ++i) {
                    row[meta->getColumnLabel(i)] = res->getString(i);
                }
                result.push_back(row);
            }
            delete res;
            delete stmt;
        }
        catch (sql::SQLException& e) {
            std::cerr << "查询错误: " << e.what() << std::endl;
        }
        return result;
    }

    int executeUpdate(const std::string& sql) {
        try {
            sql::Statement* stmt = conn->createStatement();
            int affected = stmt->executeUpdate(sql);
            delete stmt;
            return affected;
        }
        catch (sql::SQLException& e) {
            std::cerr << "更新错误: " << e.what() << std::endl;
            return -1;
        }
    }

    uint64_t lastInsertId() {
        sql::Statement* stmt = conn->createStatement();
        sql::ResultSet* res = stmt->executeQuery("SELECT LAST_INSERT_ID()");
        uint64_t id = 0;
        if (res->next()) id = res->getUInt64(1);
        delete res;
        delete stmt;
        return id;
    }
};

static MySQLHelper* g_db = nullptr;

bool initDatabase(const std::string& host, unsigned int port,
    const std::string& user, const std::string& pass,
    const std::string& dbName) {
    try {
        g_db = new MySQLHelper(host, port, user, pass, dbName);
        return true;
    }
    catch (...) {
        return false;
    }
}

void cleanupDatabase() {
    delete g_db;
    g_db = nullptr;
}

std::string generateToken(int userId, const std::string& username) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(100000, 999999);
    int randomNum = dis(gen);
    time_t now = std::time(nullptr);
    return std::to_string(userId) + "|" + std::to_string(now) + "|" + std::to_string(randomNum);
}

bool verifyToken(const std::string& token, int& userId, std::string& username) {
    size_t first = token.find('|');
    size_t second = token.find('|', first + 1);
    if (first == std::string::npos || second == std::string::npos) return false;
    try {
        userId = std::stoi(token.substr(0, first));
        time_t timestamp = std::stol(token.substr(first + 1, second - first - 1));
        time_t now = std::time(nullptr);
        if (now - timestamp > 24 * 3600) return false;
        json rows = g_db->executeQuery("SELECT username FROM users WHERE id = " + std::to_string(userId));
        if (rows.empty()) return false;
        username = rows[0]["username"].template get<std::string>();
        return true;
    }
    catch (...) {
        return false;
    }
}

time_t nowTimestamp() { return std::time(nullptr); }

std::string timestampToDate(time_t ts) {
    std::tm tm_buf;
#ifdef _WIN32
    localtime_s(&tm_buf, &ts);
#else
    localtime_r(&ts, &tm_buf);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%d");
    return oss.str();
}

std::string timestampToDateTime(time_t ts) {
    std::tm tm_buf;
#ifdef _WIN32
    localtime_s(&tm_buf, &ts);
#else
    localtime_r(&ts, &tm_buf);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M");
    return oss.str();
}

json handleLoginLogic(const std::string& username, const std::string& password) {
    json result;
    if (username.empty() || password.empty()) {
        result["success"] = false;
        result["message"] = "Username and password cannot be empty";
        return result;
    }
    std::string sql = "SELECT id, username, password_hash FROM users WHERE username = '" + username + "'";
    json rows = g_db->executeQuery(sql);
    if (rows.empty()) {
        result["success"] = false;
        result["message"] = "Invalid username or password";
        return result;
    }
    std::string db_pwd = rows[0]["password_hash"].template get<std::string>();
    if (password != db_pwd) {
        result["success"] = false;
        result["message"] = "Invalid username or password";
        return result;
    }
    int userId = std::stoi(rows[0]["id"].template get<std::string>());
    std::string dbUsername = rows[0]["username"].template get<std::string>();
    g_db->executeUpdate("UPDATE users SET last_login = " + std::to_string(nowTimestamp()) + ", interview_count = interview_count + 1 WHERE id = " + std::to_string(userId));
    std::string token = generateToken(userId, dbUsername);
    result["success"] = true;
    result["message"] = "Login successful";
    result["token"] = token;
    return result;
}

json handleRegisterLogic(const std::string& username, const std::string& email, const std::string& password) {
    json result;
    std::regex usernameRegex("^[a-zA-Z0-9_]{3,20}$");
    std::regex emailRegex("([^\\s@]+)@([^\\s@]+)\\.([^\\s@]+)");
    if (!std::regex_match(username, usernameRegex)) {
        result["success"] = false;
        result["message"] = "Username must be 3-20 alphanumeric or underscore";
        return result;
    }
    if (!std::regex_match(email, emailRegex)) {
        result["success"] = false;
        result["message"] = "Invalid email format";
        return result;
    }
    if (password.length() < 6) {
        result["success"] = false;
        result["message"] = "Password must be at least 6 characters";
        return result;
    }
    time_t now = nowTimestamp();
    std::string avatarInitial = username.substr(0, 1);
    std::string sql = "INSERT INTO users (username, password_hash, email, avatar_initial, register_time, last_login) VALUES ('" +
        username + "', '" + password + "', '" + email + "', '" + avatarInitial + "', " +
        std::to_string(now) + ", " + std::to_string(now) + ")";
    if (g_db->executeUpdate(sql) < 0) {
        result["success"] = false;
        result["message"] = "Username or email already exists";
        return result;
    }
    int userId = (int)g_db->lastInsertId();
    std::string token = generateToken(userId, username);
    result["success"] = true;
    result["message"] = "Registration successful";
    result["token"] = token;
    return result;
}

json getProfileLogic(int userId) {
    json result;
    if (userId == -1) {
        result["error"] = "Unauthorized";
        return result;
    }
    std::string sql = "SELECT username, email, phone, bio, register_time, last_login, interview_count, avatar_initial FROM users WHERE id = " + std::to_string(userId);
    json rows = g_db->executeQuery(sql);
    if (rows.empty()) {
        result["error"] = "User not found";
        return result;
    }
    json profile = rows[0];
    profile["registerTime"] = timestampToDate(std::stol(profile["register_time"].template get<std::string>()));
    profile["lastLogin"] = timestampToDateTime(std::stol(profile["last_login"].template get<std::string>()));
    profile.erase("register_time");
    profile.erase("last_login");
    json skills = g_db->executeQuery("SELECT skill_name FROM user_skills WHERE user_id = " + std::to_string(userId));
    json skillList = json::array();
    for (auto& s : skills) skillList.push_back(s["skill_name"].template get<std::string>());
    profile["skills"] = skillList;
    return profile;
}

bool updateProfileLogic(int userId, const std::string& email, const std::string& phone, const std::string& bio) {
    if (userId == -1) return false;
    std::string sql = "UPDATE users SET email = '" + email + "', phone = '" + phone + "', bio = '" + bio + "' WHERE id = " + std::to_string(userId);
    return g_db->executeUpdate(sql) >= 0;
}

json getHistoryLogic(int userId) {
    json result;
    if (userId == -1) {
        result["error"] = "Unauthorized";
        return result;
    }
    std::string sql = "SELECT company, position, score, date, status, stage, comment, strengths, improvements FROM interview_history WHERE user_id = " + std::to_string(userId) + " ORDER BY date DESC";
    json rows = g_db->executeQuery(sql);
    for (auto& row : rows) {
        row["date"] = timestampToDate(std::stol(row["date"].template get<std::string>()));
        try { if (!row["strengths"].is_null()) row["strengths"] = json::parse(row["strengths"].template get<std::string>()); }
        catch (...) { row["strengths"] = json::array(); }
        try { if (!row["improvements"].is_null()) row["improvements"] = json::parse(row["improvements"].template get<std::string>()); }
        catch (...) { row["improvements"] = json::array(); }
    }
    return rows;
}

// ==================== 修复：100% 匹配前端需要的 JSON 格式 ====================
json getRadarLogic() {
    json radar;
    radar["indicators"] = { "System Design", "Coding Ability", "Algorithm", "Problem Solving",
                           "Team Management", "Product Thinking", "Communication", "Time Management" };
    radar["values"] = { 9.2, 8.8, 8.5, 8.3, 6.5, 7.0, 7.2, 7.1 };

    // 前端必须的字段，已补齐
    radar["strengths"] = R"([
        {"name":"系统设计","score":9.2,"trend":"+0.5"},
        {"name":"编码能力","score":8.8,"trend":"+0.3"}
    ])"_json;

    radar["weaknesses"] = R"([
        {"name":"团队管理","score":6.5,"trend":"-0.2"},
        {"name":"产品思维","score":7.0,"trend":"-0.1"}
    ])"_json;

    return radar;
}

json getTrendLogic() {
    json trend;
    trend["categories"] = { "5th", "4th", "3rd", "2nd", "1st" };
    json series = json::array();
    json item;
    item["name"] = "Overall Score";
    item["data"] = { 8.5, 8.8, 8.2, 8.9, 8.6 };
    series.push_back(item);
    trend["series"] = series;
    return trend;
}

json getSuggestionsLogic() {
    json rows = g_db->executeQuery("SELECT suggestion FROM suggestions ORDER BY id");
    json suggestions = json::array();
    for (auto& row : rows)
        suggestions.push_back(row["suggestion"].template get<std::string>());
    json result;
    result["suggestions"] = suggestions;
    return result;
}

json getJobsLogic() {
    json rows = g_db->executeQuery("SELECT * FROM dev_test_001");
    json result;
    result["data"] = rows;
    return result;
}

// ==================== 预留：技能接口实现（已注释，前端启用后打开） ====================
/*
bool addUserSkill(int userId, const std::string& skill) {
    if (userId == -1 || skill.empty()) return false;
    std::string sql = "INSERT INTO user_skills (user_id, skill_name) VALUES (" + std::to_string(userId) + ", '" + skill + "')";
    return g_db->executeUpdate(sql) >= 0;
}

bool deleteUserSkill(int userId, const std::string& skill) {
    if (userId == -1 || skill.empty()) return false;
    std::string sql = "DELETE FROM user_skills WHERE user_id = " + std::to_string(userId) + " AND skill_name = '" + skill + "'";
    return g_db->executeUpdate(sql) >= 0;
}
*/