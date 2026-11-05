#include "db_user.h"
#include "index_dbtest.cpp"
#include <sstream>
#include <ctime>

static MySQLHelper* getDB() {
    static MySQLHelper db("192.0.2.1", 50306,
        "Invoke User", "qige_060803",
        "htzy_data");
    return &db;
}

bool initUserDatabase() {
    std::string createUsers = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INT AUTO_INCREMENT PRIMARY KEY,
            username VARCHAR(50) UNIQUE NOT NULL,
            password_hash VARCHAR(255) NOT NULL,
            email VARCHAR(100),
            phone VARCHAR(20),
            avatar VARCHAR(255),
            bio TEXT,
            register_time DATETIME DEFAULT CURRENT_TIMESTAMP,
            last_login DATETIME,
            interview_count INT DEFAULT 0
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
    )";
    std::string createSkills = R"(
        CREATE TABLE IF NOT EXISTS skills (
            id INT AUTO_INCREMENT PRIMARY KEY,
            user_id INT NOT NULL,
            skill_name VARCHAR(50) NOT NULL,
            FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
            UNIQUE KEY (user_id, skill_name)
        ) ENGINE=InnoDB;
    )";
    std::string createResume = R"(
        CREATE TABLE IF NOT EXISTS resume (
            user_id INT PRIMARY KEY,
            content_html LONGTEXT,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
            FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
        ) ENGINE=InnoDB;
    )";
    std::string createHistory = R"(
        CREATE TABLE IF NOT EXISTS interview_history (
            id INT AUTO_INCREMENT PRIMARY KEY,
            user_id INT NOT NULL,
            company VARCHAR(100),
            position VARCHAR(100),
            score DECIMAL(3,1),
            date DATE,
            status VARCHAR(20),
            stage VARCHAR(50),
            comment TEXT,
            strengths TEXT,
            improvements TEXT,
            FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
        ) ENGINE=InnoDB;
    )";
    std::string createAbility = R"(
        CREATE TABLE IF NOT EXISTS ability_scores (
            id INT AUTO_INCREMENT PRIMARY KEY,
            user_id INT NOT NULL,
            dimension VARCHAR(50),
            score DECIMAL(3,1),
            evaluate_date DATE,
            FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
        ) ENGINE=InnoDB;
    )";

    MySQLHelper* db = getDB();
    if (db->executeUpdate(createUsers) < 0) return false;
    if (db->executeUpdate(createSkills) < 0) return false;
    if (db->executeUpdate(createResume) < 0) return false;
    if (db->executeUpdate(createHistory) < 0) return false;
    if (db->executeUpdate(createAbility) < 0) return false;
    return true;
}

bool registerUser(const std::string& username, const std::string& password,
    const std::string& email, const std::string& phone, int& userId) {
    MySQLHelper* db = getDB();
    std::string sql = "INSERT INTO users (username, password_hash, email, phone) VALUES ('"
        + username + "', '" + password + "', '" + email + "', '" + phone + "')";
    int affected = db->executeUpdate(sql);
    if (affected > 0) {
        std::string idSql = "SELECT LAST_INSERT_ID()";
        std::string res = db->queryData(idSql);
        if (!res.empty()) {
            json j = json::parse(db->convertToJson(res));
            if (!j["data"].empty()) {
                userId = std::stoi(j["data"][0]["LAST_INSERT_ID()"].get<std::string>());
            }
        }
        return true;
    }
    return false;
}

int loginUser(const std::string& username, const std::string& password) {
    MySQLHelper* db = getDB();
    std::string sql = "SELECT id, password_hash FROM users WHERE username = '" + username + "'";
    std::string res = db->queryData(sql);
    if (res.empty()) return -1;
    json j = json::parse(db->convertToJson(res));
    if (j["data"].empty()) return -1;
    int id = std::stoi(j["data"][0]["id"].get<std::string>());
    std::string hash = j["data"][0]["password_hash"];
    if (hash == password) {
        std::string updateSql = "UPDATE users SET last_login = NOW() WHERE id = " + std::to_string(id);
        db->executeUpdate(updateSql);
        return id;
    }
    return -1;
}

json getUserProfile(int userId) {
    MySQLHelper* db = getDB();
    std::string sql = "SELECT id, username, email, phone, bio, register_time, last_login, interview_count FROM users WHERE id = " + std::to_string(userId);
    std::string res = db->queryData(sql);
    if (res.empty()) return json::object();
    json j = json::parse(db->convertToJson(res));
    if (j["data"].empty()) return json::object();
    return j["data"][0];
}

bool updateUserProfile(int userId, const json& data) {
    MySQLHelper* db = getDB();
    std::string sql = "UPDATE users SET ";
    bool first = true;
    if (data.contains("username")) {
        sql += (first ? "" : ", ") + std::string("username='") + data["username"].get<std::string>() + "'";
        first = false;
    }
    if (data.contains("email")) {
        sql += (first ? "" : ", ") + std::string("email='") + data["email"].get<std::string>() + "'";
        first = false;
    }
    if (data.contains("phone")) {
        sql += (first ? "" : ", ") + std::string("phone='") + data["phone"].get<std::string>() + "'";
        first = false;
    }
    if (data.contains("bio")) {
        sql += (first ? "" : ", ") + std::string("bio='") + data["bio"].get<std::string>() + "'";
        first = false;
    }
    if (first) return false;
    sql += " WHERE id = " + std::to_string(userId);
    return db->executeUpdate(sql) >= 0;
}

std::vector<std::string> getSkills(int userId) {
    MySQLHelper* db = getDB();
    std::string sql = "SELECT skill_name FROM skills WHERE user_id = " + std::to_string(userId);
    std::string res = db->queryData(sql);
    if (res.empty()) return {};
    json j = json::parse(db->convertToJson(res));
    std::vector<std::string> skills;
    for (auto& row : j["data"]) {
        skills.push_back(row["skill_name"]);
    }
    return skills;
}

bool addSkill(int userId, const std::string& skill) {
    MySQLHelper* db = getDB();
    std::string sql = "INSERT INTO skills (user_id, skill_name) VALUES (" + std::to_string(userId) + ", '" + skill + "')";
    return db->executeUpdate(sql) > 0;
}

bool removeSkill(int userId, const std::string& skill) {
    MySQLHelper* db = getDB();
    std::string sql = "DELETE FROM skills WHERE user_id = " + std::to_string(userId) + " AND skill_name = '" + skill + "'";
    return db->executeUpdate(sql) > 0;
}

std::string getResume(int userId) {
    MySQLHelper* db = getDB();
    std::string sql = "SELECT content_html FROM resume WHERE user_id = " + std::to_string(userId);
    std::string res = db->queryData(sql);
    if (res.empty()) return "<h2>Default Resume</h2><p>No content</p>";
    json j = json::parse(db->convertToJson(res));
    if (j["data"].empty()) return "";
    return j["data"][0]["content_html"];
}

bool updateResume(int userId, const std::string& content) {
    MySQLHelper* db = getDB();
    std::string escaped = content;
    size_t pos = 0;
    while ((pos = escaped.find("'", pos)) != std::string::npos) {
        escaped.replace(pos, 1, "\\'");
        pos += 2;
    }
    std::string sql = "INSERT INTO resume (user_id, content_html) VALUES (" + std::to_string(userId) + ", '" + escaped + "') ON DUPLICATE KEY UPDATE content_html = '" + escaped + "'";
    return db->executeUpdate(sql) >= 0;
}

json getHistory(int userId, int page, int pageSize, const std::string& keyword) {
    MySQLHelper* db = getDB();
    int offset = (page - 1) * pageSize;
    std::string sql = "SELECT company, position, score, date, status, stage, comment, strengths, improvements FROM interview_history WHERE user_id = " + std::to_string(userId);
    if (!keyword.empty()) {
        sql += " AND (company LIKE '%" + keyword + "%' OR position LIKE '%" + keyword + "%')";
    }
    sql += " ORDER BY date DESC LIMIT " + std::to_string(offset) + ", " + std::to_string(pageSize);
    std::string res = db->queryData(sql);
    json result;
    if (res.empty()) {
        result["data"] = json::array();
        result["total"] = 0;
    }
    else {
        json j = json::parse(db->convertToJson(res));
        result["data"] = j["data"];
        result["total"] = j["total"];
    }
    return result;
}

// 修正：使用英文键名，避免中文硬编码
json getAnalysisData(int userId, const std::string& timeRange) {
    MySQLHelper* db = getDB();
    std::string sql = "SELECT dimension, score FROM ability_scores WHERE user_id = " + std::to_string(userId) + " ORDER BY evaluate_date DESC LIMIT 8";
    std::string res = db->queryData(sql);
    json data;
    if (!res.empty()) {
        json j = json::parse(db->convertToJson(res));
        std::vector<std::string> indicators;
        std::vector<double> values;
        for (auto& row : j["data"]) {
            indicators.push_back(row["dimension"]);
            values.push_back(std::stod(row["score"].get<std::string>()));
        }
        data["radar"]["indicators"] = indicators;
        data["radar"]["values"] = values;
    }
    else {
        // 使用英文标识符，前端会映射为中文
        data["radar"]["indicators"] = { "System Design", "Coding", "Algorithm", "Problem Solving", "Team Management", "Product Thinking", "Communication", "Time Management" };
        data["radar"]["values"] = { 9.2, 8.8, 8.5, 8.3, 6.5, 7.0, 7.2, 7.1 };
    }
    data["trend"]["categories"] = { "5th", "4th", "3rd", "2nd", "1st" };
    data["trend"]["series"] = { {{"name", "Overall"}, {"data", {8.5, 8.8, 8.2, 8.9, 8.6}}} };
    data["strengths"] = {
        {{"name", "System Design"}, {"score", 9.2}, {"trend", "+0.5"}},
        {{"name", "Coding"}, {"score", 8.8}, {"trend", "+0.3"}}
    };
    data["weaknesses"] = {
        {{"name", "Team Management"}, {"score", 6.5}, {"trend", "+0.8"}},
        {{"name", "Product Thinking"}, {"score", 7.0}, {"trend", "-0.2"}}
    };
    data["suggestions"] = {
        "Team Management: Attend leadership training courses.",
        "Product Thinking: Participate more in product requirement discussions."
    };
    return data;
}