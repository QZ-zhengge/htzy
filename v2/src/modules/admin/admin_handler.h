// Admin Handler - student management, scores, export, super admin
#pragma once

#include <string>
#include <sstream>
#include <vector>
#include "../../common/logger.h"
#include "../../third_party/json.hpp"
#include "../../db/database.h"
#include "../auth/auth_handler.h"

namespace admin {

// Helper: check if user is admin or super_admin
static bool isAdmin(const std::string& uid) {
    Database& db = Database::instance();
    Rows r = db.query("SELECT role FROM users WHERE id=" + uid);
    return !r.empty() && (r[0]["role"] == "admin" || r[0]["role"] == "super_admin");
}

static bool isSuperAdmin(const std::string& uid) {
    Database& db = Database::instance();
    Rows r = db.query("SELECT role FROM users WHERE id=" + uid);
    return !r.empty() && r[0]["role"] == "super_admin";
}

static void registerRoutes(httplib::Server& svr, const std::string& jwt_secret) {

    // ========== Super Admin: Add Admin ==========
    svr.Post("/api/admin/super/add-admin", [&jwt_secret](const httplib::Request& req, httplib::Response& res) {
        std::string uid = auth::extractUserId(req, jwt_secret);
        if (uid.empty() || !isSuperAdmin(uid)) {
            res.status = 403; res.set_content(R"({"error":"Forbidden: super_admin only"})", "application/json"); return;
        }
        nlohmann::json body;
        try { body = nlohmann::json::parse(req.body); }
        catch (...) { res.status = 400; res.set_content(R"({"error":"Invalid JSON"})", "application/json"); return; }

        std::string username = body.value("username", "");
        std::string password = body.value("password", "");
        std::string email = body.value("email", "");
        if (username.size() < 3 || password.size() < 6 || email.empty()) {
            res.status = 400; res.set_content(R"({"error":"username 3-20 chars, password 6+, email required"})", "application/json"); return;
        }

        Database& db = Database::instance();
        std::string safe_user = sqlEscape(username);
        std::string safe_email = sqlEscape(email);

        Rows exist = db.query("SELECT id FROM users WHERE username='" + safe_user + "'");
        if (!exist.empty()) { res.status = 409; res.set_content(R"({"error":"Username already exists"})", "application/json"); return; }

        std::string salt = auth::generateSalt();
        std::string hash = auth::sha256(salt + password);

        std::string sql = "INSERT INTO users (username, password_hash, salt, email, role) VALUES ('" +
            safe_user + "','" + hash + "','" + salt + "','" + safe_email + "','admin')";
        if (!db.execute(sql)) {
            res.status = 500; res.set_content(R"({"error":"Create admin failed"})", "application/json"); return;
        }
        uint64_t new_id = db.lastInsertId();
        nlohmann::json r;
        r["success"] = true; r["admin_id"] = new_id; r["username"] = username;
        res.set_content(r.dump(), "application/json");
    });

    // ========== Students: Upload roster from CSV ==========
    svr.Post("/api/admin/students/upload-excel", [&jwt_secret](const httplib::Request& req, httplib::Response& res) {
        std::string uid = auth::extractUserId(req, jwt_secret);
        if (uid.empty() || !isAdmin(uid)) {
            res.status = 403; res.set_content("{\"error\":\"Forbidden\"}", "application/json"); return;
        }

        // Parse CSV from request body (Content-Type: text/csv or text/plain)
        std::string body = req.body;
        if (body.empty()) {
            res.status = 400; res.set_content("{\"error\":\"No data uploaded\"}", "application/json"); return;
        }

        Database& db = Database::instance();
        int imported = 0, updated = 0;
        std::istringstream ss(body);
        std::string line;
        bool first = true;

        while (std::getline(ss, line)) {
            if (line.empty()) continue;
            if (first) { first = false; continue; } // skip header

            // Split by comma (simple CSV, no quoted fields)
            std::vector<std::string> fields;
            std::istringstream fs(line);
            std::string field;
            while (std::getline(fs, field, ',')) {
                // Trim
                size_t s = field.find_first_not_of(" \t\r\n");
                size_t e = field.find_last_not_of(" \t\r\n");
                fields.push_back((s != std::string::npos) ? field.substr(s, e - s + 1) : "");
            }

            if (fields.size() < 2 || fields[0].empty() || fields[1].empty()) continue;

            std::string sid = sqlEscape(fields[0]);
            std::string name = sqlEscape(fields[1]);
            std::string cls = fields.size() > 2 ? sqlEscape(fields[2]) : "";
            std::string dept = fields.size() > 3 ? sqlEscape(fields[3]) : "";

            Rows exist = db.query("SELECT id FROM students WHERE student_id='" + sid + "'");
            if (exist.empty()) {
                db.execute("INSERT INTO students (student_id, real_name, class_name, department) VALUES ('" +
                    sid + "','" + name + "','" + cls + "','" + dept + "')");
                imported++;
            } else {
                db.execute("UPDATE students SET real_name='" + name + "', class_name='" + cls +
                    "', department='" + dept + "' WHERE student_id='" + sid + "'");
                updated++;
            }
        }
        nlohmann::json r;
        r["success"] = true; r["imported"] = imported; r["updated"] = updated;
        r["total"] = imported + updated;
        res.set_content(r.dump(), "application/json");
    });

    // ========== Students: Upload roster (JSON array) - kept for backward compat ==========
    svr.Post("/api/admin/students/upload", [&jwt_secret](const httplib::Request& req, httplib::Response& res) {
        std::string uid = auth::extractUserId(req, jwt_secret);
        if (uid.empty() || !isAdmin(uid)) {
            res.status = 403; res.set_content(R"({"error":"Forbidden"})", "application/json"); return;
        }
        nlohmann::json body;
        try { body = nlohmann::json::parse(req.body); }
        catch (...) { res.status = 400; res.set_content(R"({"error":"Invalid JSON"})", "application/json"); return; }

        if (!body.is_array()) {
            res.status = 400; res.set_content(R"({"error":"Body must be a JSON array"})", "application/json"); return;
        }

        Database& db = Database::instance();
        int imported = 0, updated = 0;
        for (auto& item : body) {
            std::string sid = item.value("student_id", "");
            std::string name = item.value("real_name", "");
            std::string cls = item.value("class_name", "");
            std::string dept = item.value("department", "");
            if (sid.empty() || name.empty()) continue;

            std::string safe_sid = sqlEscape(sid);
            std::string safe_name = sqlEscape(name);
            std::string safe_cls = sqlEscape(cls);
            std::string safe_dept = sqlEscape(dept);

            // Upsert: insert or update
            Rows exist = db.query("SELECT id FROM students WHERE student_id='" + safe_sid + "'");
            if (exist.empty()) {
                db.execute("INSERT INTO students (student_id, real_name, class_name, department) VALUES ('" +
                    safe_sid + "','" + safe_name + "','" + safe_cls + "','" + safe_dept + "')");
                imported++;
            } else {
                db.execute("UPDATE students SET real_name='" + safe_name + "', class_name='" + safe_cls +
                    "', department='" + safe_dept + "' WHERE student_id='" + safe_sid + "'");
                updated++;
            }
        }
        nlohmann::json r;
        r["success"] = true; r["imported"] = imported; r["updated"] = updated;
        res.set_content(r.dump(), "application/json");
    });

    // ========== Students: List roster ==========
    svr.Get("/api/admin/students", [&jwt_secret](const httplib::Request& req, httplib::Response& res) {
        std::string uid = auth::extractUserId(req, jwt_secret);
        if (uid.empty() || !isAdmin(uid)) {
            res.status = 403; res.set_content(R"({"error":"Forbidden"})", "application/json"); return;
        }
        Database& db = Database::instance();

        int page = 1, size = 50;
        if (req.has_param("page")) page = std::max(1, std::stoi(req.get_param_value("page")));
        if (req.has_param("size")) size = std::clamp(std::stoi(req.get_param_value("size")), 1, 200);
        int offset = (page - 1) * size;

        std::string where = "1=1";
        if (req.has_param("keyword") && !req.get_param_value("keyword").empty()) {
            std::string kw = sqlEscape(req.get_param_value("keyword"));
            where += " AND (student_id LIKE '%" + kw + "%' OR real_name LIKE '%" + kw + "%')";
        }

        Rows count_rows = db.query("SELECT COUNT(*) AS total FROM students WHERE " + where);
        int total = count_rows.empty() ? 0 : std::stoi(count_rows[0]["total"]);

        Rows rows = db.query("SELECT * FROM students WHERE " + where + " ORDER BY id ASC LIMIT " +
            std::to_string(size) + " OFFSET " + std::to_string(offset));

        nlohmann::json result;
        result["total"] = total; result["page"] = page; result["size"] = size;
        nlohmann::json list = nlohmann::json::array();
        for (auto& r : rows) {
            nlohmann::json item;
            item["id"] = std::stoi(r["id"]);
            item["student_id"] = r["student_id"];
            item["real_name"] = r["real_name"];
            item["class_name"] = r["class_name"];
            item["department"] = r["department"];
            list.push_back(item);
        }
        result["data"] = list;
        res.set_content(result.dump(), "application/json");
    });

    // ========== Students: Registered status ==========
    svr.Get("/api/admin/students/registered", [&jwt_secret](const httplib::Request& req, httplib::Response& res) {
        std::string uid = auth::extractUserId(req, jwt_secret);
        if (uid.empty() || !isAdmin(uid)) {
            res.status = 403; res.set_content(R"({"error":"Forbidden"})", "application/json"); return;
        }
        Database& db = Database::instance();
        Rows rows = db.query(
            "SELECT u.id, u.username, u.student_id, u.real_name, u.class_name, u.email, "
            "u.is_verified, u.created_at, u.last_login, u.interview_count "
            "FROM users u WHERE u.role='student' ORDER BY u.created_at DESC");

        nlohmann::json list = nlohmann::json::array();
        for (auto& r : rows) {
            nlohmann::json item;
            item["id"] = std::stoi(r["id"]);
            item["username"] = r["username"];
            item["student_id"] = r["student_id"];
            item["real_name"] = r["real_name"];
            item["class_name"] = r["class_name"];
            item["email"] = r["email"];
            item["is_verified"] = r["is_verified"] == "1";
            item["created_at"] = r["created_at"];
            item["last_login"] = r["last_login"];
            item["interview_count"] = std::stoi(r["interview_count"]);
            list.push_back(item);
        }
        res.set_content(list.dump(), "application/json");
    });

    // ========== Scores: All student scores ==========
    svr.Get("/api/admin/scores", [&jwt_secret](const httplib::Request& req, httplib::Response& res) {
        std::string uid = auth::extractUserId(req, jwt_secret);
        if (uid.empty() || !isAdmin(uid)) {
            res.status = 403; res.set_content(R"({"error":"Forbidden"})", "application/json"); return;
        }
        Database& db = Database::instance();

        Rows rows = db.query(
            "SELECT u.id as user_id, u.username, u.student_id, u.real_name, u.class_name, "
            "i.id as interview_id, i.job_position, i.status, i.created_at, "
            "r.professional_score, r.language_score, r.expression_score, r.overall_score "
            "FROM users u "
            "JOIN interviews i ON u.id = i.user_id "
            "LEFT JOIN interview_results r ON i.id = r.interview_id "
            "WHERE u.role='student' "
            "ORDER BY u.student_id, i.created_at DESC");

        nlohmann::json list = nlohmann::json::array();
        for (auto& r : rows) {
            nlohmann::json item;
            item["student_id"] = r["student_id"];
            item["real_name"] = r["real_name"];
            item["class_name"] = r["class_name"];
            item["username"] = r["username"];
            item["job_position"] = r["job_position"];
            item["status"] = r["status"];
            item["created_at"] = r["created_at"];
            item["professional_score"] = r["professional_score"];
            item["language_score"] = r["language_score"];
            item["expression_score"] = r["expression_score"];
            item["overall_score"] = r["overall_score"];
            list.push_back(item);
        }
        res.set_content(list.dump(), "application/json");
    });

    // ========== Scores: Export as CSV ==========
    svr.Get("/api/admin/export/scores", [&jwt_secret](const httplib::Request& req, httplib::Response& res) {
        std::string uid = auth::extractUserId(req, jwt_secret);
        if (uid.empty() || !isAdmin(uid)) {
            res.status = 403; res.set_content(R"({"error":"Forbidden"})", "application/json"); return;
        }
        Database& db = Database::instance();
        Rows rows = db.query(
            "SELECT u.student_id, u.real_name, u.class_name, u.username, "
            "i.job_position, i.created_at, "
            "r.professional_score, r.language_score, r.expression_score, r.overall_score "
            "FROM users u "
            "JOIN interviews i ON u.id = i.user_id "
            "LEFT JOIN interview_results r ON i.id = r.interview_id "
            "WHERE u.role='student' AND r.id IS NOT NULL "
            "ORDER BY u.student_id, i.created_at DESC");

        std::string csv = "\xEF\xBB\xBF"; // UTF-8 BOM for Excel
        csv += "\xe5\xad\xa6\xe5\x8f\xb7,\xe5\x90\x8d\xe5\xad\x97,\xe7\x8f\xad\xe7\xba\xa7,\xe7\x94\xa8\xe6\x88\xb7\xe5\x90\x8d,\xe5\xb2\x97\xe4\xbd\x8d,\xe9\x9d\xa2\xe8\xaf\x95\xe6\x97\xb6\xe9\x97\xb4,\xe4\xb8\x93\xe4\xb8\x9a\xe8\x83\xbd\xe5\x8a\x9b,\xe8\xaf\xad\xe8\xa8\x80\xe8\xa1\xa8\xe8\xbe\xbe,\xe8\xa1\xa8\xe6\x83\x85\xe7\xae\xa1\xe7\x90\x86,\xe6\x80\xbb\xe5\x88\x86\n";
        for (auto& r : rows) {
            csv += r["student_id"] + "," + r["real_name"] + "," + r["class_name"] + "," +
                   r["username"] + "," + r["job_position"] + "," + r["created_at"] + "," +
                   r["professional_score"] + "," + r["language_score"] + "," +
                   r["expression_score"] + "," + r["overall_score"] + "\n";
        }
        res.set_content(csv, "text/csv; charset=utf-8");
        res.set_header("Content-Disposition", "attachment; filename=\"scores.csv\"");
    });

    // ========== Stats: Dashboard overview ==========
    svr.Get("/api/admin/stats", [&jwt_secret](const httplib::Request& req, httplib::Response& res) {
        std::string uid = auth::extractUserId(req, jwt_secret);
        if (uid.empty() || !isAdmin(uid)) {
            res.status = 403; res.set_content(R"({"error":"Forbidden"})", "application/json"); return;
        }
        Database& db = Database::instance();

        Rows total_students = db.query("SELECT COUNT(*) AS c FROM students");
        Rows registered = db.query("SELECT COUNT(*) AS c FROM users WHERE role='student'");
        Rows verified = db.query("SELECT COUNT(*) AS c FROM users WHERE role='student' AND is_verified=1");
        Rows total_interviews = db.query("SELECT COUNT(*) AS c FROM interviews");
        Rows completed = db.query("SELECT COUNT(*) AS c FROM interviews WHERE status='completed'");
        Rows avg_score = db.query("SELECT AVG(overall_score) AS avg FROM interview_results");

        nlohmann::json r;
        r["total_students"] = total_students.empty() ? 0 : std::stoi(total_students[0]["c"]);
        r["registered"] = registered.empty() ? 0 : std::stoi(registered[0]["c"]);
        r["verified"] = verified.empty() ? 0 : std::stoi(verified[0]["c"]);
        r["total_interviews"] = total_interviews.empty() ? 0 : std::stoi(total_interviews[0]["c"]);
        r["completed_interviews"] = completed.empty() ? 0 : std::stoi(completed[0]["c"]);
        r["average_score"] = avg_score.empty() || avg_score[0]["avg"].empty() ? 0 : std::stod(avg_score[0]["avg"]);
        res.set_content(r.dump(), "application/json");
    });

    LOG_INFO("Admin", "Routes registered: super/add-admin, students CRUD, scores, export, stats");
}

} // namespace admin
