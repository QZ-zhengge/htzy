// RedStory Handler - list / detail for red culture stories
#pragma once

#include <string>
#include "../../common/logger.h"
#include "../../third_party/json.hpp"
#include "../../db/database.h"

namespace redstory {

static void registerRoutes(httplib::Server& svr) {

    // GET /api/redstories - list all stories
    svr.Get("/api/redstories", [](const httplib::Request& req, httplib::Response& res) {
        Database& db = Database::instance();
        Rows rows = db.query("SELECT id, title, summary, category, source_name, view_count, created_at "
            "FROM red_stories ORDER BY id ASC");
        nlohmann::json list = nlohmann::json::array();
        for (auto& r : rows) {
            nlohmann::json item;
            item["id"] = std::stoull(r["id"]);
            item["title"] = r["title"];
            item["summary"] = r["summary"];
            item["category"] = r["category"];
            item["source_name"] = r["source_name"];
            item["view_count"] = std::stoi(r["view_count"]);
            item["created_at"] = r["created_at"];
            list.push_back(item);
        }
        res.set_content(list.dump(), "application/json");
    });

    // GET /api/redstories/:id - detail with full content
    svr.Get(R"(/api/redstories/(\d+))", [](const httplib::Request& req, httplib::Response& res) {
        std::string sid = req.matches[1];
        Database& db = Database::instance();
        Rows rows = db.query("SELECT * FROM red_stories WHERE id=" + sid);
        if (rows.empty()) {
            res.status = 404;
            res.set_content(R"({"error":"Story not found"})", "application/json");
            return;
        }
        // Increment view count
        db.execute("UPDATE red_stories SET view_count = view_count + 1 WHERE id=" + sid);

        auto& r = rows[0];
        nlohmann::json item;
        item["id"] = std::stoull(r["id"]);
        item["title"] = r["title"];
        item["summary"] = r["summary"];
        item["content"] = r["content"];
        item["category"] = r["category"];
        item["image_url"] = r["image_url"];
        item["source_url"] = r["source_url"];
        item["source_name"] = r["source_name"];
        item["view_count"] = std::stoi(r["view_count"]) + 1;
        item["created_at"] = r["created_at"];
        res.set_content(item.dump(), "application/json");
    });

    LOG_INFO("RedStory", "Routes registered: GET /api/redstories, GET /api/redstories/:id");
}

} // namespace redstory
