// HTTP Server - httplib wrapper + route registration
#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <functional>
#include "../../third_party/httplib.h"
#include "../common/config.h"
#include "../common/logger.h"
#include "../modules/auth/auth_handler.h"
#include "../modules/user/user_handler.h"
#include "../modules/resume/resume_handler.h"
#include "../modules/jobs/jobs_handler.h"
#include "../modules/interview/interview_handler.h"
#include "../modules/analysis/analysis_handler.h"
#include "../modules/redstory/redstory_handler.h"
#include "../modules/admin/admin_handler.h"
#include "../modules/delivery/delivery_handler.h"

class HttpServer {
public:
    static HttpServer& instance() {
        static HttpServer inst;
        return inst;
    }

    void setStaticDir(const std::string& dir) { static_dir_ = dir; }

    httplib::Server& server() { return svr_; }

    void registerRoutes(const std::string& jwt_secret) {
        // Health check
        svr_.Get("/api/health", [](const httplib::Request&, httplib::Response& res) {
            res.set_content(R"({"status":"ok","service":"HTZY v2.0"})", "application/json");
        });

        auth::registerAll(svr_, jwt_secret);
        user::registerRoutes(svr_, jwt_secret);
        resume::registerRoutes(svr_, jwt_secret);
        jobs::registerRoutes(svr_);
        interview::registerRoutes(svr_, jwt_secret);
        analysis::registerRoutes(svr_, jwt_secret);
        redstory::registerRoutes(svr_);
        admin::registerRoutes(svr_, jwt_secret);
        delivery::registerRoutes(svr_, jwt_secret);

        LOG_INFO("HTTP", "All routes registered");
    }

    bool start(const std::string& host, int port) {
        if (!static_dir_.empty()) {
            svr_.set_mount_point("/", static_dir_);
            LOG_INFO("HTTP", "Static dir: " + static_dir_);
        }

        // Enable multi-threading for concurrent requests (50+ users)
        svr_.new_task_queue = [] {
            return new httplib::ThreadPool(8, 16);  // 8 threads, max 16 queued
        };

        svr_.set_default_headers({
            {"Access-Control-Allow-Origin", "*"},
            {"Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS"},
            {"Access-Control-Allow-Headers", "Content-Type, Authorization"}
        });

        svr_.Options(".*", [](const httplib::Request&, httplib::Response& res) {
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
            res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
            res.status = 204;
        });

        // SPA fallback: any non-API GET that doesn't match a static file -> return index.html
        // This fixes Vue Router history mode: /login, /red-guide/1 etc. all serve index.html
        std::string index_html;
        if (!static_dir_.empty()) {
            std::ifstream ifs(static_dir_ + "/index.html", std::ios::binary);
            if (ifs.is_open()) {
                std::ostringstream oss;
                oss << ifs.rdbuf();
                index_html = oss.str();
            }
        }

        if (!index_html.empty()) {
            svr_.set_error_handler([this, index_html](const httplib::Request& req, httplib::Response& res) {
                // Only fallback for GET requests to non-API paths
                if (req.method == "GET" && res.status == 404 &&
                    req.path.find("/api/") == std::string::npos) {
                    res.status = 200;
                    res.set_content(index_html, "text/html; charset=utf-8");
                }
            });
        }

        LOG_INFO("HTTP", "Listening on http://" + host + ":" + std::to_string(port));
        if (!index_html.empty()) {
            LOG_INFO("HTTP", "SPA fallback enabled (Vue Router history mode)");
        }
        return svr_.listen(host, port);
    }

    void stop() { svr_.stop(); }

private:
    HttpServer() = default;
    httplib::Server svr_;
    std::string static_dir_;
};
