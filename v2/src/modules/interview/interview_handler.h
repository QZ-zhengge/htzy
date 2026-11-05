// Interview Handler - create / start / answer / end / results
// Full interview lifecycle, mirrors Python app.py flow
#pragma once

#include <string>
#include <vector>
#include "../../common/logger.h"
#include "../../third_party/json.hpp"
#include "../../db/database.h"
#include "../auth/auth_handler.h"
#include "../../ai/deepseek_client.h"

namespace interview {

// Helper: compute weighted overall score
static double computeOverall(double professional, double language, double expression) {
    const auto& cfg = Config::instance().get().interview;
    return professional * cfg.weight_professional +
           language * cfg.weight_language +
           expression * cfg.weight_expression;
}

static void registerRoutes(httplib::Server& svr, const std::string& jwt_secret) {

    // POST /api/interviews - Create interview + AI generate questions
    svr.Post("/api/interviews", [&jwt_secret](const httplib::Request& req, httplib::Response& res) {
        std::string uid = auth::extractUserId(req, jwt_secret);
        if (uid.empty()) { res.status = 401; res.set_content(R"({"error":"Unauthorized"})", "application/json"); return; }

        nlohmann::json body;
        try { body = nlohmann::json::parse(req.body); }
        catch (...) { res.status = 400; res.set_content(R"({"error":"Invalid JSON"})", "application/json"); return; }

        // Accept both job_position and jobPosition from frontend
        std::string job_position = body.value("job_position", body.value("jobPosition", ""));
        if (job_position.empty()) {
            res.status = 400;
            res.set_content(R"({"error":"Missing job_position"})", "application/json");
            return;
        }

        Database& db = Database::instance();

        // Check user has at least one resume
        Rows resumes = db.query("SELECT id FROM resumes WHERE user_id=" + uid + " ORDER BY upload_time DESC LIMIT 1");
        if (resumes.empty()) {
            res.status = 400;
            res.set_content(R"({"error":"Please upload a resume first"})", "application/json");
            return;
        }

        // Create interview record
        std::string sql = "INSERT INTO interviews (user_id, job_position, status) VALUES (" +
            uid + ",'" + sqlEscape(job_position) + "','pending')";
        if (!db.execute(sql)) {
            res.status = 500;
            res.set_content(R"({"error":"Failed to create interview"})", "application/json");
            return;
        }
        uint64_t interview_id = db.lastInsertId();
        LOG_INFO("Interview", "Created interview #" + std::to_string(interview_id));

        // Get resume content for AI analysis
        Rows resume_rows = db.query("SELECT content FROM resumes WHERE user_id=" + uid + " ORDER BY upload_time DESC LIMIT 1");
        std::string resume_content = resume_rows.empty() ? "" : resume_rows[0]["content"];

        // AI: Analyze resume
        DeepseekClient& ai = DeepseekClient::instance();
        json resume_info = ai.analyzeResume(resume_content, job_position);

        // AI: Generate questions
        int q_count = Config::instance().get().interview.question_count;
        json questions = ai.generateQuestions(resume_info, job_position, q_count);

        int saved_count = 0;
        if (questions.is_object() && questions.contains("questions")) {
            for (int i = 0; i < static_cast<int>(questions["questions"].size()); ++i) {
                auto& q = questions["questions"][i];
                std::string q_text = q.value("text", "");
                std::string q_type = q.value("type", "technical");
                int difficulty = q.value("difficulty", 3);
                if (q_text.empty()) continue;

                std::string q_sql = "INSERT INTO interview_questions (interview_id, question_text, question_type, difficulty, order_number) VALUES (" +
                    std::to_string(interview_id) + ",'" + sqlEscape(q_text) + "','" + sqlEscape(q_type) + "'," +
                    std::to_string(difficulty) + "," + std::to_string(i + 1) + ")";
                if (db.execute(q_sql)) saved_count++;
            }
        }

        LOG_INFO("Interview", "Saved " + std::to_string(saved_count) + " questions for #" + std::to_string(interview_id));

        nlohmann::json result;
        result["success"] = true;
        result["interview_id"] = interview_id;
        result["question_count"] = saved_count;
        res.set_content(result.dump(), "application/json");
    });

    // GET /api/interviews - list user's interviews
    svr.Get("/api/interviews", [&jwt_secret](const httplib::Request& req, httplib::Response& res) {
        std::string uid = auth::extractUserId(req, jwt_secret);
        if (uid.empty()) { res.status = 401; res.set_content(R"({"error":"Unauthorized"})", "application/json"); return; }

        Database& db = Database::instance();
        // Join with results to get score
        Rows rows = db.query(
            "SELECT i.id, i.job_position, i.status, i.start_time, i.end_time, i.created_at, "
            "r.overall_score FROM interviews i "
            "LEFT JOIN interview_results r ON i.id = r.interview_id "
            "WHERE i.user_id=" + uid + " ORDER BY i.created_at DESC");
        nlohmann::json list = nlohmann::json::array();
        for (auto& r : rows) {
            nlohmann::json item;
            item["id"] = std::stoull(r["id"]);
            item["job_position"] = r["job_position"];
            item["status"] = r["status"];
            item["start_time"] = r["start_time"];
            item["end_time"] = r["end_time"];
            item["created_at"] = r["created_at"];
            if (!r["overall_score"].empty())
                item["overall_score"] = r["overall_score"];
            list.push_back(item);
        }
        res.set_content(list.dump(), "application/json");
    });

    // GET /api/interviews/:id - detail with questions
    svr.Get(R"(/api/interviews/(\d+))", [&jwt_secret](const httplib::Request& req, httplib::Response& res) {
        std::string uid = auth::extractUserId(req, jwt_secret);
        if (uid.empty()) { res.status = 401; res.set_content(R"({"error":"Unauthorized"})", "application/json"); return; }

        std::string iid = req.matches[1];
        Database& db = Database::instance();
        Rows rows = db.query("SELECT * FROM interviews WHERE id=" + iid + " AND user_id=" + uid);
        if (rows.empty()) { res.status = 404; res.set_content(R"({"error":"Not found"})", "application/json"); return; }

        nlohmann::json detail;
        detail["id"] = std::stoull(rows[0]["id"]);
        detail["job_position"] = rows[0]["job_position"];
        detail["status"] = rows[0]["status"];
        detail["start_time"] = rows[0]["start_time"];
        detail["end_time"] = rows[0]["end_time"];

        // Get questions
        Rows qs = db.query("SELECT id, question_text, question_type, difficulty, order_number "
            "FROM interview_questions WHERE interview_id=" + iid + " ORDER BY order_number");
        nlohmann::json qlist = nlohmann::json::array();
        for (auto& q : qs) {
            nlohmann::json qi;
            qi["id"] = std::stoull(q["id"]);
            qi["text"] = q["question_text"];
            qi["type"] = q["question_type"];
            qi["difficulty"] = std::stoi(q["difficulty"]);
            qi["order"] = std::stoi(q["order_number"]);
            qlist.push_back(qi);
        }
        detail["questions"] = qlist;

        // Get result if exists
        Rows rr = db.query("SELECT * FROM interview_results WHERE interview_id=" + iid);
        if (!rr.empty()) {
            nlohmann::json ri;
            ri["professional_score"] = rr[0]["professional_score"];
            ri["language_score"] = rr[0]["language_score"];
            ri["expression_score"] = rr[0]["expression_score"];
            ri["overall_score"] = rr[0]["overall_score"];
            ri["report"] = rr[0]["report"];
            detail["result"] = ri;
        }

        res.set_content(detail.dump(), "application/json");
    });

    // POST /api/interviews/:id/start
    svr.Post(R"(/api/interviews/(\d+)/start)", [&jwt_secret](const httplib::Request& req, httplib::Response& res) {
        std::string uid = auth::extractUserId(req, jwt_secret);
        if (uid.empty()) { res.status = 401; res.set_content(R"({"error":"Unauthorized"})", "application/json"); return; }

        std::string iid = req.matches[1];
        Database& db = Database::instance();
        // Atomic: only transition from pending to in_progress
        bool ok = db.execute("UPDATE interviews SET status='in_progress', start_time=NOW() WHERE id=" + iid + " AND user_id=" + uid + " AND status='pending'");
        if (!ok) {
            // Check why it failed
            Rows rows = db.query("SELECT id, status FROM interviews WHERE id=" + iid + " AND user_id=" + uid);
            if (rows.empty()) {
                res.status = 404;
                res.set_content(R"({"error":"Not found"})", "application/json");
            } else {
                res.status = 400;
                res.set_content(R"({"error":"Interview already started or completed"})", "application/json");
            }
            return;
        }
        res.set_content(R"({"success":true,"message":"Interview started"})", "application/json");
    });

    // POST /api/interviews/:id/answers - submit answer for a question
    svr.Post(R"(/api/interviews/(\d+)/answers)", [&jwt_secret](const httplib::Request& req, httplib::Response& res) {
        std::string uid = auth::extractUserId(req, jwt_secret);
        if (uid.empty()) { res.status = 401; res.set_content(R"({"error":"Unauthorized"})", "application/json"); return; }

        std::string iid = req.matches[1];
        nlohmann::json body;
        try { body = nlohmann::json::parse(req.body); }
        catch (...) { res.status = 400; res.set_content(R"({"error":"Invalid JSON"})", "application/json"); return; }

        int question_id = body.value("question_id", 0);
        std::string answer_text = body.value("answer_text", "");
        if (question_id <= 0 || answer_text.empty()) {
            res.status = 400;
            res.set_content(R"({"error":"Missing question_id or answer_text"})", "application/json");
            return;
        }

        Database& db = Database::instance();
        // Check interview belongs to user and is in_progress
        Rows rows = db.query("SELECT id, status, job_position FROM interviews WHERE id=" + iid + " AND user_id=" + uid);
        if (rows.empty()) { res.status = 404; res.set_content(R"({"error":"Not found"})", "application/json"); return; }
        if (rows[0]["status"] != "in_progress") {
            res.status = 400;
            res.set_content(R"({"error":"Interview not in progress"})", "application/json");
            return;
        }

        // Check no duplicate answer
        Rows exist = db.query("SELECT id FROM interview_answers WHERE question_id=" + std::to_string(question_id));
        if (!exist.empty()) {
            res.status = 409;
            res.set_content(R"({"error":"Answer already submitted for this question"})", "application/json");
            return;
        }

        // Escape answer text
        std::string safe = sqlEscape(answer_text);

        std::string sql = "INSERT INTO interview_answers (interview_id, question_id, answer_text, start_time, end_time) VALUES (" +
            iid + "," + std::to_string(question_id) + ",'" + safe + "', NOW(), NOW())";
        if (db.execute(sql)) {
            uint64_t aid = db.lastInsertId();
            nlohmann::json r; r["success"] = true; r["answer_id"] = aid;
            res.set_content(r.dump(), "application/json");
        } else {
            res.status = 500;
            res.set_content(R"({"error":"Save failed"})", "application/json");
        }
    });

    // POST /api/interviews/:id/end - end interview, compute scores + AI report
    svr.Post(R"(/api/interviews/(\d+)/end)", [&jwt_secret](const httplib::Request& req, httplib::Response& res) {
        std::string uid = auth::extractUserId(req, jwt_secret);
        if (uid.empty()) { res.status = 401; res.set_content(R"({"error":"Unauthorized"})", "application/json"); return; }

        std::string iid = req.matches[1];
        Database& db = Database::instance();
        Rows rows = db.query("SELECT id, job_position FROM interviews WHERE id=" + iid + " AND user_id=" + uid);
        if (rows.empty()) { res.status = 404; res.set_content(R"({"error":"Not found"})", "application/json"); return; }
        // Check and atomically mark completed (prevents double-end)
        bool ok = db.execute("UPDATE interviews SET status='completed', end_time=NOW() WHERE id=" + iid + " AND user_id=" + uid + " AND status='in_progress'");
        if (!ok) {
            Rows check = db.query("SELECT id, status FROM interviews WHERE id=" + iid + " AND user_id=" + uid);
            if (check.empty()) { res.status = 404; res.set_content(R"({"error":"Not found"})", "application/json"); return; }
            res.status = 400;
            res.set_content(R"({"error":"Interview not in progress or already completed"})", "application/json");
            return;
        }

        // Check if result already exists (idempotent)
        Rows existing_result = db.query("SELECT id FROM interview_results WHERE interview_id=" + iid);
        if (!existing_result.empty()) {
            // Already scored, return existing result
            Rows rr = db.query("SELECT * FROM interview_results WHERE interview_id=" + iid);
            nlohmann::json result;
            result["success"] = true;
            result["overall_score"] = std::stod(rr[0]["overall_score"]);
            result["professional_score"] = std::stod(rr[0]["professional_score"]);
            result["language_score"] = std::stod(rr[0]["language_score"]);
            result["expression_score"] = std::stod(rr[0]["expression_score"]);
            res.set_content(result.dump(), "application/json");
            return;
        }

        // Get questions and answers
        Rows questions = db.query("SELECT id, question_text FROM interview_questions WHERE interview_id=" + iid + " ORDER BY order_number");
        Rows answers = db.query("SELECT question_id, answer_text FROM interview_answers WHERE interview_id=" + iid);

        // Build question->answer map
        std::map<std::string, std::string> qa_map;
        for (auto& a : answers) qa_map[a["question_id"]] = a["answer_text"];

        // AI: analyze each answer
        DeepseekClient& ai = DeepseekClient::instance();
        std::vector<double> professional_scores;
        std::string job_position = rows[0]["job_position"];

        for (auto& q : questions) {
            auto it = qa_map.find(q["id"]);
            if (it == qa_map.end()) continue;
            json analysis = ai.analyzeAnswer(q["question_text"], it->second, job_position);
            if (analysis.is_object() && analysis.contains("professional_score")) {
                professional_scores.push_back(analysis["professional_score"].get<double>());
            }
        }

        double professional_avg = 0;
        if (!professional_scores.empty()) {
            double sum = 0;
            for (double s : professional_scores) sum += s;
            professional_avg = sum / professional_scores.size();
        }

        // Get facial analysis averages
        Rows facial = db.query(
            "SELECT AVG(stress_level) as avg_stress, AVG(dressing_neat) as avg_dressing, "
            "AVG(eye_contact) as avg_eye, AVG(facial_express) as avg_face "
            "FROM facial_analysis WHERE interview_id=" + iid
        );
        double expression_score = 0;
        if (!facial.empty() && !facial[0]["avg_stress"].empty() && facial[0]["avg_stress"] != "NULL") {
            try {
                expression_score = (std::stod(facial[0]["avg_stress"]) + std::stod(facial[0]["avg_dressing"]) +
                                   std::stod(facial[0]["avg_eye"]) + std::stod(facial[0]["avg_face"])) / 4.0;
            } catch (...) { expression_score = 0; }
        }

        // Get voice analysis averages
        Rows voice = db.query(
            "SELECT AVG(mandarin) as avg_m, AVG(voice_stability) as avg_vs, "
            "AVG(clarity) as avg_c, AVG(organization) as avg_o "
            "FROM voice_analysis WHERE interview_id=" + iid
        );
        double language_score = 0;
        if (!voice.empty() && !voice[0]["avg_m"].empty() && voice[0]["avg_m"] != "NULL") {
            try {
                language_score = (std::stod(voice[0]["avg_m"]) + std::stod(voice[0]["avg_vs"]) +
                                 std::stod(voice[0]["avg_c"]) + std::stod(voice[0]["avg_o"])) / 4.0;
            } catch (...) { language_score = 0; }
        }

        double overall = computeOverall(professional_avg, language_score, expression_score);

        // AI: Generate report
        json interview_data;
        interview_data["job_position"] = job_position;
        json facial_j, voice_j, prof_j;
        if (!facial.empty()) {
            facial_j["stress"] = facial[0]["avg_stress"];
            facial_j["dressing"] = facial[0]["avg_dressing"];
            facial_j["eye_contact"] = facial[0]["avg_eye"];
            facial_j["facial"] = facial[0]["avg_face"];
        }
        if (!voice.empty()) {
            voice_j["mandarin"] = voice[0]["avg_m"];
            voice_j["stability"] = voice[0]["avg_vs"];
            voice_j["clarity"] = voice[0]["avg_c"];
            voice_j["organization"] = voice[0]["avg_o"];
        }
        prof_j["average"] = professional_avg;
        prof_j["details"] = professional_scores;

        json report = ai.generateReport(interview_data, facial_j, voice_j, prof_j);
        std::string report_str = report.is_object() ? report.dump() : std::string("{}");

        // Save result
        std::string esc_report = sqlEscape(report_str);

        std::string sql = "INSERT INTO interview_results (interview_id, professional_score, language_score, "
            "expression_score, overall_score, report) VALUES (" +
            iid + "," + std::to_string(professional_avg) + "," + std::to_string(language_score) + "," +
            std::to_string(expression_score) + "," + std::to_string(overall) + ",'" + esc_report + "')";
        db.execute(sql);

        LOG_INFO("Interview", "Interview #" + iid + " ended. Overall: " + std::to_string(overall));

        nlohmann::json result;
        result["success"] = true;
        result["overall_score"] = overall;
        result["professional_score"] = professional_avg;
        result["language_score"] = language_score;
        result["expression_score"] = expression_score;
        res.set_content(result.dump(), "application/json");
    });

    // GET /api/interviews/:id/results
    svr.Get(R"(/api/interviews/(\d+)/results)", [&jwt_secret](const httplib::Request& req, httplib::Response& res) {
        std::string uid = auth::extractUserId(req, jwt_secret);
        if (uid.empty()) { res.status = 401; res.set_content(R"({"error":"Unauthorized"})", "application/json"); return; }

        std::string iid = req.matches[1];
        Database& db = Database::instance();
        // Verify ownership
        Rows check = db.query("SELECT id FROM interviews WHERE id=" + iid + " AND user_id=" + uid);
        if (check.empty()) { res.status = 404; res.set_content(R"({"error":"Not found"})", "application/json"); return; }

        Rows rr = db.query("SELECT * FROM interview_results WHERE interview_id=" + iid);
        if (rr.empty()) { res.status = 404; res.set_content(R"({"error":"Results not yet available"})", "application/json"); return; }

        nlohmann::json result;
        result["interview_id"] = std::stoull(iid);
        result["professional_score"] = rr[0]["professional_score"];
        result["language_score"] = rr[0]["language_score"];
        result["expression_score"] = rr[0]["expression_score"];
        result["overall_score"] = rr[0]["overall_score"];
        result["report"] = rr[0]["report"];
        result["created_at"] = rr[0]["created_at"];
        res.set_content(result.dump(), "application/json");
    });

    LOG_INFO("Interview", "Routes registered: CRUD /api/interviews, start/end/answers/results");
}

} // namespace interview
