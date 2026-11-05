// Auth Handler - register / login / JWT middleware
#pragma once

#include <string>
#include <sstream>
#include "../../common/mailer.h"
#include "../../common/verify_code.h"
#include <random>
#include <iomanip>
#include "../../common/config.h"
#include "../../common/logger.h"
#include "../../common/jwt.h"
#include "../../third_party/json.hpp"
#include "../../db/database.h"

// Forward declaration to avoid circular include
class HttpServer;

namespace auth {

static std::string sha256_str(const std::string& input) {
    // Get raw SHA-256 binary, then convert to hex string
    std::string raw = jwt::sha256(input);
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (unsigned char c : raw) {
        oss << std::setw(2) << static_cast<int>(c);
    }
    return oss.str();
}

static std::string sha256(const std::string& input) {
    return sha256_str(input);
}

static std::string generateSalt(int len = 16) {
    static const char chars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, sizeof(chars) - 2);
    std::string s;
    s.reserve(len);
    for (int i = 0; i < len; ++i) s += chars[dis(gen)];
    return s;
}

static std::string extractUserId(const httplib::Request& req, const std::string& jwt_secret) {
    auto auth = req.get_header_value("Authorization");
    if (auth.empty()) return "";
    if (auth.find("Bearer ") != 0) return "";
    std::string token = auth.substr(7);
    return jwt::verify(token, jwt_secret);
}

static void registerAll(httplib::Server& svr, const std::string& jwt_secret) {
    // POST /api/auth/register - requires student_id + real_name + class_name
    svr.Post("/api/auth/register", [&jwt_secret](const httplib::Request& req, httplib::Response& res) {
        nlohmann::json body;
        try { body = nlohmann::json::parse(req.body); }
        catch (...) { res.status = 400; res.set_content(R"({"error":"Invalid JSON"})", "application/json"); return; }

        // Require email_verified flag (user must call /api/auth/verify-code first)
        if (!body.value("email_verified", false)) {
            res.status = 400;
            res.set_content(R"({"error":"Please verify your email first"})", "application/json");
            return;
        }

        if (!body.contains("username") || !body.contains("password") || !body.contains("email") ||
            !body.contains("student_id") || !body.contains("real_name")) {
            res.status = 400;
            res.set_content(R"({"error":"Missing username/password/email/student_id/real_name"})", "application/json");
            return;
        }
        std::string username = body["username"].get<std::string>();
        std::string password = body["password"].get<std::string>();
        std::string email = body["email"].get<std::string>();
        std::string student_id = body["student_id"].get<std::string>();
        std::string real_name = body["real_name"].get<std::string>();
        std::string class_name = body.value("class_name", "");

        if (username.size() < 3 || username.size() > 20 || password.size() < 6) {
            res.status = 400;
            res.set_content(R"({"error":"Username 3-20 chars, password 6+ chars"})", "application/json");
            return;
        }

        Database& db = Database::instance();
        std::string safe_user = sqlEscape(username);
        std::string safe_email = sqlEscape(email);
        std::string safe_sid = sqlEscape(student_id);
        std::string safe_name = sqlEscape(real_name);
        std::string safe_cls = sqlEscape(class_name);

        // Check username uniqueness
        Rows exist = db.query("SELECT id FROM users WHERE username='" + safe_user + "'");
        if (!exist.empty()) {
            res.status = 409;
            res.set_content(R"({"error":"Username already exists"})", "application/json");
            return;
        }
        // Check email uniqueness
        exist = db.query("SELECT id FROM users WHERE email='" + safe_email + "'");
        if (!exist.empty()) {
            res.status = 409;
            res.set_content(R"({"error":"Email already exists"})", "application/json");
            return;
        }
        // Check student_id not already registered
        exist = db.query("SELECT id FROM users WHERE student_id='" + safe_sid + "'");
        if (!exist.empty()) {
            res.status = 409;
            res.set_content(R"({"error":"This student ID is already registered"})", "application/json");
            return;
        }

        // Verify against students roster
        Rows roster = db.query("SELECT id, real_name FROM students WHERE student_id='" + safe_sid + "'");
        int is_verified = 0;
        if (roster.empty()) {
            // Student not in roster - allow but mark unverified
            is_verified = 0;
        } else {
            // Student exists in roster - verify name matches
            std::string roster_name = roster[0]["real_name"];
            if (roster_name == real_name) {
                is_verified = 1;
            } else {
                res.status = 400;
                res.set_content(R"({"error":"Name does not match student record. Please check your name."})", "application/json");
                return;
            }
        }

        std::string salt = generateSalt();
        std::string hash = sha256(salt + password);

        std::string sql = "INSERT INTO users (username, password_hash, salt, email, student_id, real_name, class_name, role, is_verified) VALUES ('" +
            safe_user + "','" + hash + "','" + salt + "','" + safe_email + "','" + safe_sid + "','" +
            safe_name + "','" + safe_cls + "','student'," + std::to_string(is_verified) + ")";
        if (!db.execute(sql)) {
            res.status = 500;
            res.set_content(R"({"error":"Registration failed"})", "application/json");
            return;
        }

        uint64_t uid = db.lastInsertId();
        std::string token = jwt::create(std::to_string(uid), username, jwt_secret);

        nlohmann::json result;
        result["success"] = true;
        result["user_id"] = uid;
        result["token"] = token;
        res.set_content(result.dump(), "application/json");
    });

    // POST /api/auth/login
    svr.Post("/api/auth/login", [&jwt_secret](const httplib::Request& req, httplib::Response& res) {
        nlohmann::json body;
        try { body = nlohmann::json::parse(req.body); }
        catch (...) { res.status = 400; res.set_content(R"({"error":"Invalid JSON"})", "application/json"); return; }

        if (!body.contains("username") || !body.contains("password")) {
            res.status = 400;
            res.set_content(R"({"error":"Missing username/password"})", "application/json");
            return;
        }
        std::string username = body["username"].get<std::string>();
        std::string password = body["password"].get<std::string>();

        Database& db = Database::instance();
        std::string safe_user = sqlEscape(username);
        Rows rows = db.query("SELECT id, password_hash, salt, role, is_verified FROM users WHERE username='" + safe_user + "'");
        if (rows.empty()) {
            res.status = 401;
            res.set_content(R"({"error":"Invalid credentials"})", "application/json");
            return;
        }

        std::string computed = sha256(rows[0]["salt"] + password);
        if (computed != rows[0]["password_hash"]) {
            res.status = 401;
            res.set_content(R"({"error":"Invalid credentials"})", "application/json");
            return;
        }

        db.execute("UPDATE users SET last_login=NOW(), interview_count=interview_count+1 WHERE id=" + rows[0]["id"]);

        std::string token = jwt::create(rows[0]["id"], username, jwt_secret);
        nlohmann::json result;
        result["success"] = true;
        result["user_id"] = std::stoull(rows[0]["id"]);
        result["username"] = username;
        result["token"] = token;
        result["role"] = rows[0]["role"];
        result["is_verified"] = rows[0]["is_verified"] == "1";
        res.set_content(result.dump(), "application/json");
    });

    // POST /api/auth/send-code - send verification code to email
    svr.Post("/api/auth/send-code", [](const httplib::Request& req, httplib::Response& res) {
        nlohmann::json body;
        try { body = nlohmann::json::parse(req.body); }
        catch (...) { res.status = 400; res.set_content(R"({"error":"Invalid JSON"})", "application/json"); return; }

        std::string email = body.value("email", "");
        if (email.empty() || email.find('@') == std::string::npos) {
            res.status = 400; res.set_content(R"({"error":"Valid email required"})", "application/json"); return;
        }

        Database& db = Database::instance();
        Rows exist = db.query("SELECT id FROM users WHERE email='" + sqlEscape(email) + "'");
        if (!exist.empty()) {
            res.status = 409; res.set_content(R"({"error":"Email already registered"})", "application/json"); return;
        }

        std::string code = VerifyCode::instance().generate(email);
        bool sent = Mailer::instance().sendVerifyCode(email, code);

        nlohmann::json r;
        r["success"] = true;
        r["message"] = sent ? "Verification code sent" : "Code generated (email sending unavailable)";
        // In dev mode, include code in response for testing
        #ifdef _DEBUG
        r["debug_code"] = code;
        #endif
        res.set_content(r.dump(), "application/json");
    });

    // POST /api/auth/verify-code - verify email code (returns temp token for registration)
    svr.Post("/api/auth/verify-code", [](const httplib::Request& req, httplib::Response& res) {
        nlohmann::json body;
        try { body = nlohmann::json::parse(req.body); }
        catch (...) { res.status = 400; res.set_content(R"({"error":"Invalid JSON"})", "application/json"); return; }

        std::string email = body.value("email", "");
        std::string code = body.value("code", "");

        if (email.empty() || code.empty()) {
            res.status = 400; res.set_content(R"({"error":"Email and code required"})", "application/json"); return;
        }

        bool ok = VerifyCode::instance().verify(email, code);
        nlohmann::json r;
        r["success"] = ok;
        r["message"] = ok ? "Email verified" : "Invalid or expired code";
        res.set_content(r.dump(), "application/json");
    });

    LOG_INFO("Auth", "Routes registered: /api/auth/register, /api/auth/login, /api/auth/send-code, /api/auth/verify-code");
}

} // namespace auth
