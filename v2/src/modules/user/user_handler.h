// User Handler - profile get/update
#pragma once

#include <string>
#include "../../common/config.h"
#include "../../common/logger.h"
#include "../../third_party/json.hpp"
#include "../../db/database.h"
#include "../auth/auth_handler.h"

namespace user {

static void registerRoutes(httplib::Server& svr, const std::string& jwt_secret) {
    // GET /api/user/profile
    svr.Get("/api/user/profile", [&jwt_secret](const httplib::Request& req, httplib::Response& res) {
        std::string uid = auth::extractUserId(req, jwt_secret);
        if (uid.empty()) {
            res.status = 401;
            res.set_content(R"({"error":"Unauthorized"})", "application/json");
            return;
        }
        Database& db = Database::instance();
        Rows rows = db.query(
            "SELECT id, username, email, phone, bio, created_at, last_login, interview_count "
            "FROM users WHERE id=" + uid
        );
        if (rows.empty()) {
            res.status = 404;
            res.set_content(R"({"error":"User not found"})", "application/json");
            return;
        }
        nlohmann::json profile;
        profile["id"] = std::stoull(rows[0]["id"]);
        profile["username"] = rows[0]["username"];
        profile["email"] = rows[0]["email"];
        profile["phone"] = rows[0]["phone"];
        profile["bio"] = rows[0]["bio"];
        profile["created_at"] = rows[0]["created_at"];
        profile["last_login"] = rows[0]["last_login"];
        profile["interview_count"] = std::stoi(rows[0]["interview_count"]);
        res.set_content(profile.dump(), "application/json");
    });

    // PUT /api/user/profile
    svr.Put("/api/user/profile", [&jwt_secret](const httplib::Request& req, httplib::Response& res) {
        std::string uid = auth::extractUserId(req, jwt_secret);
        if (uid.empty()) {
            res.status = 401;
            res.set_content(R"({"error":"Unauthorized"})", "application/json");
            return;
        }
        auto body = nlohmann::json::parse(req.body, nullptr, false);
        if (!body.is_object()) {
            res.status = 400;
            res.set_content(R"({"error":"Invalid JSON"})", "application/json");
            return;
        }
        Database& db = Database::instance();
        std::string sql = "UPDATE users SET ";
        bool first = true;
        for (const auto& field : {"email", "phone", "bio"}) {
            if (body.contains(field)) {
                if (!first) sql += ",";
                sql += std::string(field) + "='" + sqlEscape(body[field].get<std::string>()) + "'";
                first = false;
            }
        }
        if (first) {
            res.status = 400;
            res.set_content(R"({"error":"No fields to update"})", "application/json");
            return;
        }
        sql += " WHERE id=" + uid;
        if (db.execute(sql)) {
            res.set_content(R"({"success":true})", "application/json");
        } else {
            res.status = 500;
            res.set_content(R"({"error":"Update failed"})", "application/json");
        }
    });

    LOG_INFO("User", "Routes registered: GET/PUT /api/user/profile");
}

} // namespace user
