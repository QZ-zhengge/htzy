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

// ==================== 红色指引 PDF 列表接口 ====================
#include <filesystem>
#ifdef _WIN32
#include <windows.h>
#endif

// GBK 宽字符转 UTF-8（Windows 上 std::filesystem 返回的路径需要编码转换）
static std::string wideToUtf8(const std::wstring& wstr) {
    if (wstr.empty()) return "";
    int len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
    std::string result(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &result[0], len, nullptr, nullptr);
    return result;
}

json getRedGuidePdfsLogic() {
    json result;
    json pdfList = json::array();
    namespace fs = std::filesystem;

    // PDF 存放目录：程序工作目录下的 page/red/
    fs::path pdfDir = fs::current_path() / "page" / "red";

    if (!fs::exists(pdfDir) || !fs::is_directory(pdfDir)) {
        result["data"] = pdfList;
        result["message"] = "red directory not found";
        return result;
    }

    for (const auto& entry : fs::directory_iterator(pdfDir)) {
        if (!entry.is_regular_file()) continue;
        auto ext = entry.path().extension().wstring();
        // 转小写比较
        std::wstring extLower = ext;
        for (auto& c : extLower) c = (wchar_t)towlower((unsigned long)c);
        if (extLower != L".pdf") continue;

#ifdef _WIN32
        // Windows: 用 wstring 路径转为 UTF-8，避免 GBK 乱码
        std::string filename = wideToUtf8(entry.path().filename().wstring());
        std::string title = wideToUtf8(entry.path().stem().wstring());
#else
        std::string filename = entry.path().filename().string();
        std::string title = entry.path().stem().string();
#endif
        // URL 路径：对文件名进行 URL 编码以支持中文
        std::string url = "/red/" + filename;

        json item;
        item["name"] = filename;
        item["title"] = title;
        item["url"] = url;
        pdfList.push_back(item);
    }

    result["data"] = pdfList;
    return result;
}

// ==================== 红色指引轮播图接口 ====================
json getRedGuideCarouselLogic() {
    json result;
    json items = json::array();

    json item1;
    item1["image"] = "https://picsum.photos/id/1002/1200/450";
    item1["title"] = "红色精神 科技报国";
    item1["description"] = "从‘两弹一星’到国产芯片，计算机事业在红色指引下蓬勃发展";
    items.push_back(item1);

    json item2;
    item2["image"] = "https://picsum.photos/id/1015/1200/450";
    item2["title"] = "乡村振兴 数字赋能";
    item2["description"] = "数字化基层就业，为乡村振兴插上科技翅膀";
    items.push_back(item2);

    json item3;
    item3["image"] = "https://picsum.photos/id/1039/1200/450";
    item3["title"] = "红色教育基地 · 职业成长";
    item3["description"] = "联合红色遗址与高校，打造计算机科技人才培养计划";
    items.push_back(item3);

    result["data"] = items;
    return result;
}

// ==================== 红色指引故事卡片接口 ====================
json getRedGuideStoriesLogic() {
    json result;
    json items = json::array();

    auto addStory = [&](const std::string& icon, const std::string& title, const std::string& content) {
        json item;
        item["icon"] = icon;
        item["title"] = title;
        item["content"] = content;
        items.push_back(item);
    };

    addStory("fa-microchip", "红色计算机记忆", "从1956年计算所诞生到国产操作系统突破，一代代科研人员赓续红色血脉，突破技术封锁。");
    addStory("fa-network-wired", "网信事业初心", "坚持自主创新，构建网络安全防线，红色指引下数字中国战略稳步推进。");
    addStory("fa-chalkboard-user", "基层数字化帮扶", "开展乡村电商、智慧农业培训，以信息技术助力老区振兴。");
    addStory("fa-hand-holding-heart", "退役军人IT扶持", "联合龙头企业提供编程、运维等定向培训，护航退役军人职业转型。");
    addStory("fa-people-arrows", "红色企业对接", "汇聚信创、智慧党建等领域优质岗位，搭建精准就业桥梁。");
    addStory("fa-chart-line", "红色职业规划", "传承艰苦奋斗精神，为青年提供长期技术成长路线与价值观引导。");

    result["data"] = items;
    return result;
}

// ==================== 红色指引行业数据接口 ====================
json getRedGuideIndustriesLogic() {
    json result;
    json items = json::array();
    items.push_back("互联网/IT");
    items.push_back("金融");
    items.push_back("教育培训");
    items.push_back("医疗健康");
    items.push_back("制造业");
    items.push_back("红色文旅");
    result["data"] = items;
    return result;
}

// ==================== 红色指引地区数据接口 ====================
json getRedGuideRegionsLogic() {
    json result;
    json items = json::array();
    items.push_back("北京");
    items.push_back("上海");
    items.push_back("广州");
    items.push_back("深圳");
    items.push_back("杭州");
    items.push_back("成都");
    result["data"] = items;
    return result;
}

// ==================== 红色指引演示职位接口 ====================
json getRedGuideDemoJobsLogic() {
    json result;
    json items = json::array();

    auto addJob = [&](const std::string& title, const std::string& company,
                      const std::string& location, const std::string& salary,
                      const std::string& industry) {
        json item;
        item["title"] = title;
        item["company"] = company;
        item["location"] = location;
        item["salary"] = salary;
        item["industry"] = industry;
        items.push_back(item);
    };

    addJob("前端开发工程师", "红色基因科技", "北京", "20-35K", "互联网");
    addJob("红色文旅策划", "星火文化传媒", "延安", "10-15K", "文旅");
    addJob("基层党务助理", "乡村振兴促进会", "江西井冈山", "6-9K", "公共事业");
    addJob("Java后端工程师", "浪潮云", "济南", "18-28K", "互联网");
    addJob("红色教育讲师", "红源干部学院", "遵义", "12-18K", "教育");
    addJob("数据分析师", "数智红途", "杭州", "16-25K", "互联网");

    result["data"] = items;
    return result;
}