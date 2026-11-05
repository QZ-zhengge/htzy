// Analysis Handler - face analysis (DeepSeek Vision) + voice analysis (DeepSeek text)
#pragma once

#include <string>
#include "../../common/logger.h"
#include "../../third_party/json.hpp"
#include "../../db/database.h"
#include "../auth/auth_handler.h"
#include "../../ai/deepseek_client.h"

namespace analysis {

static void registerRoutes(httplib::Server& svr, const std::string& jwt_secret) {

    // POST /api/interviews/:id/analyze_face - camera frame analysis
    // Body: {"frame_number":1,"timestamp":1234567890,"image":"base64..."}
    svr.Post(R"(/api/interviews/(\d+)/analyze_face)", [&jwt_secret](const httplib::Request& req, httplib::Response& res) {
        std::string uid = auth::extractUserId(req, jwt_secret);
        if (uid.empty()) { res.status = 401; res.set_content(R"({"error":"Unauthorized"})", "application/json"); return; }

        std::string iid = req.matches[1];
        nlohmann::json body;
        try { body = nlohmann::json::parse(req.body); }
        catch (...) { res.status = 400; res.set_content(R"({"error":"Invalid JSON"})", "application/json"); return; }

        if (!body.contains("frame_number") || !body.contains("image")) {
            res.status = 400;
            res.set_content(R"({"error":"Missing frame_number or image"})", "application/json");
            return;
        }

        Database& db = Database::instance();
        Rows rows = db.query("SELECT id, status FROM interviews WHERE id=" + iid + " AND user_id=" + uid);
        if (rows.empty()) { res.status = 404; res.set_content(R"({"error":"Not found"})", "application/json"); return; }
        if (rows[0]["status"] != "in_progress") {
            // Return zero scores if interview not active
            res.set_content(R"({"stress":0,"dressing":0,"eye_contact":0,"facial":0})", "application/json");
            return;
        }

        int frame_number = body["frame_number"].get<int>();

        // TODO: Call DeepSeek Vision API (deepseek-flash) for real facial analysis
        // Accepts base64 image, returns stress/dressing/eye_contact/facial scores
        double stress = 40.0, dressing = 50.0, eye_contact = 40.0, facial = 45.0;

        // Save to DB
        std::string sql = "INSERT INTO facial_analysis (interview_id, frame_number, stress_level, dressing_neat, eye_contact, facial_express) VALUES (" +
            iid + "," + std::to_string(frame_number) + "," +
            std::to_string(stress) + "," + std::to_string(dressing) + "," +
            std::to_string(eye_contact) + "," + std::to_string(facial) + ")";
        db.execute(sql);

        nlohmann::json result;
        result["stress"] = stress;
        result["dressing"] = dressing;
        result["eye_contact"] = eye_contact;
        result["facial"] = facial;
        res.set_content(result.dump(), "application/json");
    });

    // POST /api/interviews/:id/analyze_voice - voice text analysis
    // Body: {"text":"the transcribed text..."}
    svr.Post(R"(/api/interviews/(\d+)/analyze_voice)", [&jwt_secret](const httplib::Request& req, httplib::Response& res) {
        std::string uid = auth::extractUserId(req, jwt_secret);
        if (uid.empty()) { res.status = 401; res.set_content(R"({"error":"Unauthorized"})", "application/json"); return; }

        std::string iid = req.matches[1];
        nlohmann::json body;
        try { body = nlohmann::json::parse(req.body); }
        catch (...) { res.status = 400; res.set_content(R"({"error":"Invalid JSON"})", "application/json"); return; }

        std::string text = body.value("text", "");
        if (text.empty()) {
            res.status = 400;
            res.set_content(R"({"error":"Missing text"})", "application/json");
            return;
        }

        Database& db = Database::instance();
        Rows rows = db.query("SELECT id, status FROM interviews WHERE id=" + iid + " AND user_id=" + uid);
        if (rows.empty()) { res.status = 404; res.set_content(R"({"error":"Not found"})", "application/json"); return; }
        if (rows[0]["status"] != "in_progress") {
            res.set_content(R"({"mandarin":0,"stability":0,"clarity":0,"organization":0})", "application/json");
            return;
        }

        // AI: analyze voice via text
        DeepseekClient& ai = DeepseekClient::instance();
        json analysis = ai.analyzeVoice(text);

        double mandarin = 0, stability = 0, clarity = 0, organization = 0;
        if (analysis.is_object()) {
            mandarin = analysis.value("mandarin", 0.0);
            stability = analysis.value("stability", 0.0);
            clarity = analysis.value("clarity", 0.0);
            organization = analysis.value("organization", 0.0);
        }

        std::string sql = "INSERT INTO voice_analysis (interview_id, mandarin, voice_stability, clarity, organization) VALUES (" +
            iid + "," + std::to_string(mandarin) + "," + std::to_string(stability) + "," +
            std::to_string(clarity) + "," + std::to_string(organization) + ")";
        db.execute(sql);

        nlohmann::json result;
        result["mandarin"] = mandarin;
        result["stability"] = stability;
        result["clarity"] = clarity;
        result["organization"] = organization;
        res.set_content(result.dump(), "application/json");
    });

    LOG_INFO("Analysis", "Routes registered: /api/interviews/:id/analyze_face, /analyze_voice");
}

} // namespace analysis
