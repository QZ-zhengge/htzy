#include <iostream>
#include <string>
#include "httplib.h"
#include "json.hpp"

using namespace httplib;
using json = nlohmann::json;

extern bool initDatabase(const std::string& host, unsigned int port,
    const std::string& user, const std::string& pass,
    const std::string& dbName);
extern void cleanupDatabase();
extern json handleLoginLogic(const std::string& username, const std::string& password);
extern json handleRegisterLogic(const std::string& username, const std::string& email, const std::string& password);
extern json getProfileLogic(int userId);
extern bool updateProfileLogic(int userId, const std::string& email, const std::string& phone, const std::string& bio);
extern json getHistoryLogic(int userId);
extern json getRadarLogic();
extern json getTrendLogic();
extern json getSuggestionsLogic();
extern json getJobsLogic();
extern json getRedGuidePdfsLogic();
extern json getRedGuideCarouselLogic();
extern json getRedGuideStoriesLogic();
extern json getRedGuideIndustriesLogic();
extern json getRedGuideRegionsLogic();
extern json getRedGuideDemoJobsLogic();

// 新增：技能相关接口 (前端未调用，已注释预留)
// extern bool addUserSkill(int userId, const std::string& skill);
// extern bool deleteUserSkill(int userId, const std::string& skill);

int getCurrentUserId(const Request& req) {
    auto auth = req.get_header_value("Authorization");
    if (auth.empty() || auth.find("Bearer ") != 0) return -1;
    std::string token = auth.substr(7);
    extern bool verifyToken(const std::string & token, int& userId, std::string & username);
    int userId; std::string username;
    if (verifyToken(token, userId, username)) return userId;
    return -1;
}

int server() {
    if (!initDatabase("39.104.91.101", 50306, "Invoke User", "qige_060803", "ai_interview")) {
        std::cerr << "Database connection failed!" << std::endl;
        return 1;
    }
    std::cout << "Database connection successful." << std::endl;

    Server svr;
    svr.set_mount_point("/", "./page");
    svr.set_default_headers({
        {"Access-Control-Allow-Origin", "*"},
        {"Access-Control-Allow-Methods", "GET, POST, PUT, OPTIONS, DELETE"},
        {"Access-Control-Allow-Headers", "Content-Type, Authorization"}
        });
    svr.Options(".*", [](const Request&, Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, OPTIONS, DELETE");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        res.status = 204;
        });

    // ==================== 已实现接口 ====================
    svr.Post("/api/users/login", [](const Request& req, Response& res) {
        try {
            json body = json::parse(req.body);
            std::string username = body.value("username", "");
            std::string password = body.value("password", "");
            json result = handleLoginLogic(username, password);
            if (result.contains("success") && !result["success"]) res.status = 401;
            res.set_content(result.dump(), "application/json");
        }
        catch (const std::exception& e) {
            res.status = 500;
            res.set_content(json{ {"success", false}, {"message", e.what()} }.dump(), "application/json");
        }
        });

    svr.Post("/api/users/register", [](const Request& req, Response& res) {
        try {
            json body = json::parse(req.body);
            std::string username = body.value("username", "");
            std::string email = body.value("email", "");
            std::string password = body.value("password", "");
            json result = handleRegisterLogic(username, email, password);
            if (result["success"]) {
                res.status = 200;
                res.set_redirect("/index.html");
                return;
            }
            if (result.contains("success") && !result["success"]) res.status = 400;
            res.set_content(result.dump(), "application/json");
        }
        catch (const std::exception& e) {
            res.status = 500;
            res.set_content(json{ {"success", false}, {"message", e.what()} }.dump(), "application/json");
        }
        });

    svr.Get("/user/profile", [](const Request& req, Response& res) {
        int userId = getCurrentUserId(req);
        // === 在这里添加检查 ===
        if (userId == -1) {
            res.status = 401;
            res.set_content(json{ {"error", "Unauthorized"} }.dump(), "application/json");
            return;
        }
        // 原业务逻辑（此时 userId 有效，可去掉 getProfileLogic 内部的 userId==-1 判断）
        json result = getProfileLogic(userId);
        res.set_content(result.dump(), "application/json");
        });

    svr.Put("/user/profile", [](const Request& req, Response& res) {
        int userId = getCurrentUserId(req);
        if (userId == -1) {
            res.status = 401;
            res.set_content(json{ {"error", "Unauthorized"} }.dump(), "application/json");
            return;
        }
        try {
            json body = json::parse(req.body);
            std::string email = body.value("email", "");
            std::string phone = body.value("phone", "");
            std::string bio = body.value("bio", "");
            bool ok = updateProfileLogic(userId, email, phone, bio);
            if (ok) res.set_content(json{ {"success", true} }.dump(), "application/json");
            else res.status = 500, res.set_content(json{ {"success", false} }.dump(), "application/json");
        }
        catch (...) { res.status = 400; }
        });

    svr.Get("/interview/history", [](const Request& req, Response& res) {
        int userId = getCurrentUserId(req);
        // === 添加相同检查 ===
        if (userId == -1) {
            res.status = 401;
            res.set_content(json{ {"error", "Unauthorized"} }.dump(), "application/json");
            return;
        }
        json result = getHistoryLogic(userId);
        res.set_content(result.dump(), "application/json");
        });

    svr.Get("/analysis/radar", [](const Request&, Response& res) {
        res.set_content(getRadarLogic().dump(), "application/json");
        });

    svr.Get("/analysis/trend", [](const Request&, Response& res) {
        res.set_content(getTrendLogic().dump(), "application/json");
        });

    svr.Get("/analysis/suggestions", [](const Request&, Response& res) {
        res.set_content(getSuggestionsLogic().dump(), "application/json");
        });

    svr.Get("/api/jobs", [](const Request&, Response& res) {
        res.set_content(getJobsLogic().dump(), "application/json");
        });

    // ==================== 红色指引 PDF 列表接口 ====================
    svr.Get("/api/redguide/pdfs", [](const Request&, Response& res) {
        res.set_content(getRedGuidePdfsLogic().dump(), "application/json");
        });

    // ==================== 红色指引轮播图接口 ====================
    svr.Get("/api/redguide/carousel", [](const Request&, Response& res) {
        res.set_content(getRedGuideCarouselLogic().dump(), "application/json");
        });

    // ==================== 红色指引故事卡片接口 ====================
    svr.Get("/api/redguide/stories", [](const Request&, Response& res) {
        res.set_content(getRedGuideStoriesLogic().dump(), "application/json");
        });

    // ==================== 红色指引行业数据接口 ====================
    svr.Get("/api/redguide/industries", [](const Request&, Response& res) {
        res.set_content(getRedGuideIndustriesLogic().dump(), "application/json");
        });

    // ==================== 红色指引地区数据接口 ====================
    svr.Get("/api/redguide/regions", [](const Request&, Response& res) {
        res.set_content(getRedGuideRegionsLogic().dump(), "application/json");
        });

    // ==================== 红色指引演示职位接口 ====================
    svr.Get("/api/redguide/demo-jobs", [](const Request&, Response& res) {
        res.set_content(getRedGuideDemoJobsLogic().dump(), "application/json");
        });

    // 挂载 /red 静态目录，让前端可通过 /red/xxx.pdf 直接访问 PDF 文件
    // PDF 文件实际存放在 ./page/red/ 目录下
    svr.set_mount_point("/red", "./page/red");

    // ==================== 前端未调用，已注释预留的接口 ====================
    // 技能添加
    /*
    svr.Post("/user/skills", [](const Request& req, Response& res) {
        int userId = getCurrentUserId(req);
        if (userId == -1) { res.status = 401; return; }
        try {
            json body = json::parse(req.body);
            std::string skill = body.value("skill", "");
            bool ok = addUserSkill(userId, skill);
            res.set_content(json{ {"success", ok} }.dump(), "application/json");
        } catch (...) { res.status = 400; }
    });
    */

    // 技能删除
    /*
    svr.Delete("/user/skills", [](const Request& req, Response& res) {
        int userId = getCurrentUserId(req);
        if (userId == -1) { res.status = 401; return; }
        try {
            json body = json::parse(req.body);
            std::string skill = body.value("skill", "");
            bool ok = deleteUserSkill(userId, skill);
            res.set_content(json{ {"success", ok} }.dump(), "application/json");
        } catch (...) { res.status = 400; }
    });
    */

    std::cout << "服务已启动！访问地址：http://localhost:8080\n";
    svr.listen("0.0.0.0", 8080);

    cleanupDatabase();
    return 0;
}