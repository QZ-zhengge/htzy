// Auto-delivery handler - auto resume delivery system
#pragma once

#include <string>
#include <ctime>
#include "../../common/logger.h"
#include "../../common/config.h"
#include "../../common/mailer.h"
#include "../../third_party/json.hpp"
#include "../../db/database.h"
#include "../auth/auth_handler.h"

namespace delivery {

static std::string getTodayStr() {
    time_t now = time(nullptr);
    struct tm* t = localtime(&now);
    char buf[11];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
    return buf;
}

static void registerRoutes(httplib::Server& svr, const std::string& jwt_secret) {

    svr.Post("/api/delivery/toggle", [&jwt_secret](const httplib::Request& req, httplib::Response& res) {
        std::string uid = auth::extractUserId(req, jwt_secret);
        if (uid.empty()) { res.status = 401; res.set_content("{\"error\":\"Unauthorized\"}", "application/json"); return; }

        nlohmann::json body;
        try { body = nlohmann::json::parse(req.body); }
        catch (...) { res.status = 400; res.set_content("{\"error\":\"Invalid JSON\"}", "application/json"); return; }

        bool enabled = body.value("enabled", false);
        int resume_id = body.value("resume_id", 0);
        std::string job_keywords = body.value("job_keywords", "");
        std::string location_pref = body.value("location_pref", "");

        if (enabled && resume_id == 0) {
            res.status = 400; res.set_content("{\"error\":\"resume_id required\"}", "application/json"); return;
        }

        Database& db = Database::instance();
        Rows exist = db.query("SELECT id FROM delivery_config WHERE user_id=" + uid);
        std::string sql;
        if (exist.empty()) {
            sql = "INSERT INTO delivery_config (user_id, enabled, resume_id, job_keywords, location_pref) VALUES (" +
                uid + "," + std::string(enabled ? "1" : "0") + "," + std::to_string(resume_id) + ",'" +
                sqlEscape(job_keywords) + "','" + sqlEscape(location_pref) + "')";
        } else {
            sql = "UPDATE delivery_config SET enabled=" + std::string(enabled ? "1" : "0") + ", resume_id=" +
                std::to_string(resume_id) + ", job_keywords='" + sqlEscape(job_keywords) +
                "', location_pref='" + sqlEscape(location_pref) + "' WHERE user_id=" + uid;
        }
        db.execute(sql);

        nlohmann::json r;
        r["success"] = true; r["enabled"] = enabled;
        res.set_content(r.dump(), "application/json");
    });

    svr.Get("/api/delivery/config", [&jwt_secret](const httplib::Request& req, httplib::Response& res) {
        std::string uid = auth::extractUserId(req, jwt_secret);
        if (uid.empty()) { res.status = 401; res.set_content("{\"error\":\"Unauthorized\"}", "application/json"); return; }

        Database& db = Database::instance();
        Rows rows = db.query("SELECT * FROM delivery_config WHERE user_id=" + uid);

        nlohmann::json r;
        if (rows.empty()) {
            r["enabled"] = false; r["resume_id"] = 0; r["job_keywords"] = ""; r["location_pref"] = "";
        } else {
            r["enabled"] = rows[0]["enabled"] == "1";
            r["resume_id"] = std::stoi(rows[0]["resume_id"]);
            r["job_keywords"] = rows[0]["job_keywords"];
            r["location_pref"] = rows[0]["location_pref"];
        }
        res.set_content(r.dump(), "application/json");
    });

    svr.Get("/api/delivery/logs", [&jwt_secret](const httplib::Request& req, httplib::Response& res) {
        std::string uid = auth::extractUserId(req, jwt_secret);
        if (uid.empty()) { res.status = 401; res.set_content("{\"error\":\"Unauthorized\"}", "application/json"); return; }

        Database& db = Database::instance();
        Rows rows = db.query(
            "SELECT d.*, j.job_title, j.company_name "
            "FROM delivery_logs d JOIN jobs j ON d.job_id = j.id "
            "WHERE d.user_id=" + uid + " ORDER BY d.sent_at DESC LIMIT 50");

        nlohmann::json list = nlohmann::json::array();
        for (auto& r : rows) {
            nlohmann::json item;
            item["id"] = std::stoi(r["id"]);
            item["job_title"] = r["job_title"];
            item["company_name"] = r["company_name"];
            item["company_email"] = r["company_email"];
            item["status"] = r["status"];
            item["sent_at"] = r["sent_at"];
            list.push_back(item);
        }
        res.set_content(list.dump(), "application/json");
    });

    svr.Get("/api/delivery/suggested", [&jwt_secret](const httplib::Request& req, httplib::Response& res) {
        std::string uid = auth::extractUserId(req, jwt_secret);
        if (uid.empty()) { res.status = 401; res.set_content("{\"error\":\"Unauthorized\"}", "application/json"); return; }

        Database& db = Database::instance();
        Rows cfg = db.query("SELECT * FROM delivery_config WHERE user_id=" + uid + " AND enabled=1");
        if (cfg.empty()) {
            res.set_content("{\"jobs\":[],\"message\":\"Auto delivery not enabled\"}", "application/json");
            return;
        }

        std::string keywords = cfg[0]["job_keywords"];
        std::string location = cfg[0]["location_pref"];
        std::string where = "contact_email IS NOT NULL AND contact_email != ''";
        if (!keywords.empty()) {
            std::string kw = sqlEscape(keywords);
            where += " AND (job_title LIKE '%" + kw + "%' OR industry LIKE '%" + kw + "%')";
        }
        if (!location.empty()) {
            std::string loc = sqlEscape(location);
            where += " AND (city LIKE '%" + loc + "%' OR location LIKE '%" + loc + "%')";
        }
        std::string today = getTodayStr();
        where += " AND id NOT IN (SELECT job_id FROM delivery_logs WHERE DATE(sent_at)='" + today + "' "
                 "GROUP BY job_id HAVING COUNT(*) >= 20)";

        Rows jobs = db.query("SELECT id, job_title, company_name, city, salary_text, education "
                             "FROM jobs WHERE " + where + " ORDER BY RAND() LIMIT 5");

        nlohmann::json list = nlohmann::json::array();
        for (auto& j : jobs) {
            nlohmann::json item;
            item["id"] = std::stoi(j["id"]);
            item["job_title"] = j["job_title"];
            item["company_name"] = j["company_name"];
            item["city"] = j["city"];
            item["salary_text"] = j["salary_text"];
            item["education"] = j["education"];
            list.push_back(item);
        }
        nlohmann::json r;
        r["jobs"] = list;
        res.set_content(r.dump(), "application/json");
    });

    LOG_INFO("Delivery", "Routes registered: toggle, config, logs, suggested");
}

static bool executeDelivery(const std::string& user_id) {
    Database& db = Database::instance();
    const auto& cfg = Config::instance().get().auto_delivery;

    std::string today = getTodayStr();
    Rows today_count = db.query("SELECT COUNT(*) AS c FROM delivery_logs WHERE user_id=" + user_id +
        " AND DATE(sent_at)='" + today + "'");
    int used = today_count.empty() ? 0 : std::stoi(today_count[0]["c"]);
    if (used >= cfg.daily_limit_per_student) return false;

    Rows user = db.query("SELECT real_name, email FROM users WHERE id=" + user_id);
    if (user.empty()) return false;
    std::string name = user[0]["real_name"];
    std::string email = user[0]["email"];

    Rows rcfg = db.query("SELECT resume_id, job_keywords, location_pref FROM delivery_config WHERE user_id=" + user_id + " AND enabled=1");
    if (rcfg.empty()) return false;
    int resume_id = std::stoi(rcfg[0]["resume_id"]);
    std::string kw = rcfg[0]["job_keywords"];
    std::string loc = rcfg[0]["location_pref"];

    std::string where = "contact_email IS NOT NULL AND contact_email != ''";
    if (!kw.empty()) {
        std::string safe_kw = sqlEscape(kw);
        where += " AND (job_title LIKE '%" + safe_kw + "%' OR industry LIKE '%" + safe_kw + "%')";
    }
    if (!loc.empty()) {
        std::string safe_loc = sqlEscape(loc);
        where += " AND (city LIKE '%" + safe_loc + "%' OR location LIKE '%" + safe_loc + "%')";
    }
    where += " AND id NOT IN (SELECT job_id FROM delivery_logs WHERE DATE(sent_at)='" + today + "' "
             "GROUP BY job_id HAVING COUNT(*) >= " + std::to_string(cfg.daily_limit_per_company) + ")";

    Rows jobs = db.query("SELECT id, job_title, company_name, contact_email FROM jobs WHERE " + where + " LIMIT 1");
    if (jobs.empty()) return false;

    int job_id = std::stoi(jobs[0]["id"]);
    std::string company = jobs[0]["company_name"];
    std::string position = jobs[0]["job_title"];
    std::string company_email = jobs[0]["contact_email"];

    bool sent = Mailer::instance().sendDeliveryNotice(email, name, company, position);
    std::string status = sent ? "sent" : "no_email";
    db.execute("INSERT INTO delivery_logs (user_id, resume_id, job_id, company_email, status) VALUES (" +
        user_id + "," + std::to_string(resume_id) + "," + std::to_string(job_id) + ",'" +
        sqlEscape(company_email) + "','" + status + "')");

    LOG_INFO("Delivery", "Delivered for user " + user_id + " -> " + company + " / " + position);
    return true;
}

} // namespace delivery
