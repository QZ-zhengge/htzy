// Jobs Handler - list (paginated search) / detail
#pragma once

#include <string>
#include "../../common/logger.h"
#include "../../third_party/json.hpp"
#include "../../db/database.h"

namespace jobs {

static void registerRoutes(httplib::Server& svr) {
    // GET /api/jobs?keyword=&city=&industry=&page=1&size=20
    svr.Get("/api/jobs", [](const httplib::Request& req, httplib::Response& res) {
        int page = 1, size = 20;
        if (req.has_param("page")) page = std::max(1, std::stoi(req.get_param_value("page")));
        if (req.has_param("size")) size = std::clamp(std::stoi(req.get_param_value("size")), 1, 100);
        int offset = (page - 1) * size;

        std::string where = "1=1";
        if (req.has_param("keyword") && !req.get_param_value("keyword").empty()) {
            std::string kw = sqlEscape(req.get_param_value("keyword"));
            where += " AND (job_title LIKE '%" + kw + "%' OR company_name LIKE '%" + kw + "%')";
        }
        if (req.has_param("city") && !req.get_param_value("city").empty())
            where += " AND city='" + sqlEscape(req.get_param_value("city")) + "'";
        if (req.has_param("industry") && !req.get_param_value("industry").empty())
            where += " AND industry='" + sqlEscape(req.get_param_value("industry")) + "'";

        Database& db = Database::instance();
        Rows count_rows = db.query("SELECT COUNT(*) AS total FROM jobs WHERE " + where);
        int total = count_rows.empty() ? 0 : std::stoi(count_rows[0]["total"]);

        Rows rows = db.query("SELECT id, job_title, company_name, location, city, salary_min, salary_max, "
            "salary_text, education, experience, industry, job_tags, source_platform, crawl_time "
            "FROM jobs WHERE " + where + " ORDER BY crawl_time DESC LIMIT " +
            std::to_string(size) + " OFFSET " + std::to_string(offset));

        nlohmann::json result;
        result["total"] = total;
        result["page"] = page;
        result["size"] = size;
        nlohmann::json list = nlohmann::json::array();
        for (auto& r : rows) {
            nlohmann::json item;
            item["id"] = std::stoll(r["id"]);
            item["job_title"] = r["job_title"];
            item["company_name"] = r["company_name"];
            item["location"] = r["location"];
            item["city"] = r["city"];
            if (!r["salary_min"].empty()) item["salary_min"] = std::stoi(r["salary_min"]);
            if (!r["salary_max"].empty()) item["salary_max"] = std::stoi(r["salary_max"]);
            item["salary_text"] = r["salary_text"];
            item["education"] = r["education"];
            item["experience"] = r["experience"];
            item["industry"] = r["industry"];
            item["crawl_time"] = r["crawl_time"];
            list.push_back(item);
        }
        result["data"] = list;
        res.set_content(result.dump(), "application/json");
    });

    // GET /api/jobs/:id
    svr.Get(R"(/api/jobs/(\d+))", [](const httplib::Request& req, httplib::Response& res) {
        std::string job_id = req.matches[1];
        Database& db = Database::instance();
        Rows rows = db.query("SELECT * FROM jobs WHERE id=" + job_id);
        if (rows.empty()) {
            res.status = 404;
            res.set_content(R"({"error":"Job not found"})", "application/json");
            return;
        }
        nlohmann::json item;
        for (auto& [k, v] : rows[0]) {
            if (!v.empty()) item[k] = v;
        }
        res.set_content(item.dump(), "application/json");
    });

    LOG_INFO("Jobs", "Routes registered: GET /api/jobs, GET /api/jobs/:id");
}

} // namespace jobs
