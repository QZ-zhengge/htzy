// Resume Handler - upload / list / detail / delete
// Uses JSON body with base64 content (httplib version lacks multipart)
#pragma once

#include <string>
#include <fstream>
#include "../../common/logger.h"
#include "../../third_party/json.hpp"
#include "../../db/database.h"
#include "../auth/auth_handler.h"

namespace resume {

// Simple base64 decode for file upload
static std::string b64_decode(const std::string& in) {
    static const int T[256] = {
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,
        52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,
        -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
        15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
        -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
        41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
    };
    std::string out;
    out.reserve(in.size() * 3 / 4);
    unsigned int v = 0; int b = -8;
    for (unsigned char c : in) {
        if (c == '=' || c == '\n' || c == '\r') continue;
        int d = T[c]; if (d < 0) continue;
        v = (v << 6) + d; b += 6;
        if (b >= 0) { out += static_cast<char>((v >> b) & 0xff); b -= 8; }
    }
    return out;
}

static void registerRoutes(httplib::Server& svr, const std::string& jwt_secret) {
    // POST /api/resumes - JSON body: {"filename":"x.pdf","content_base64":"..."}
    svr.Post("/api/resumes", [&jwt_secret](const httplib::Request& req, httplib::Response& res) {
        std::string uid = auth::extractUserId(req, jwt_secret);
        if (uid.empty()) { res.status = 401; res.set_content(R"({"error":"Unauthorized"})", "application/json"); return; }

        nlohmann::json body;
        try { body = nlohmann::json::parse(req.body); }
        catch (...) { res.status = 400; res.set_content(R"({"error":"Invalid JSON"})", "application/json"); return; }

        std::string filename = body.value("filename", "");
        std::string content_b64 = body.value("content_base64", "");

        if (filename.empty()) {
            res.status = 400;
            res.set_content(R"({"error":"Missing filename"})", "application/json");
            return;
        }

        // Sanitize filename: remove path separators and .. to prevent path traversal
        std::string safe_name;
        for (char c : filename) {
            if (c == '/' || c == '\\' || c == '\0') continue;
            safe_name += c;
        }
        // Remove any ".." sequences
        size_t pos;
        while ((pos = safe_name.find("..")) != std::string::npos) {
            safe_name.erase(pos, 2);
        }
        if (safe_name.empty()) safe_name = "upload.bin";

        // Save file to disk (binary) - use absolute path based on exe location
        std::string save_dir = "uploads/";
#ifdef _WIN32
        // Create uploads relative to exe directory (not CWD)
        char exe_dir[MAX_PATH];
        GetModuleFileNameA(NULL, exe_dir, MAX_PATH);
        std::string exe_path(exe_dir);
        size_t last_sep = exe_path.find_last_of("\\/");
        std::string base_dir = (last_sep != std::string::npos) ? exe_path.substr(0, last_sep + 1) : "";
        save_dir = base_dir + "uploads/";
        CreateDirectoryA(save_dir.c_str(), NULL);
#else
        mkdir(save_dir.c_str(), 0755);
#endif
        // Decode base64 for file save
        std::string content_bin = content_b64.empty() ? "" : b64_decode(content_b64);
        std::string save_path = save_dir + safe_name;
        std::ofstream ofs(save_path, std::ios::binary);
        if (ofs.is_open()) {
            ofs.write(content_bin.data(), content_bin.size());
            ofs.close();
        }

        // Save to DB - store base64 string (not decoded binary) so VARCHAR is safe
        Database& db = Database::instance();
        // Truncate very long base64 to fit in DB (50K chars max)
        std::string safe_content = content_b64;
        if (safe_content.size() > 50000) safe_content = safe_content.substr(0, 50000);

        std::string sql = "INSERT INTO resumes (user_id, file_path, content) VALUES (" +
            uid + ",'" + sqlEscape(save_path) + "','" + sqlEscape(safe_content) + "')";
        if (db.execute(sql)) {
            uint64_t rid = db.lastInsertId();
            nlohmann::json result;
            result["success"] = true;
            result["resume_id"] = rid;
            result["file_path"] = save_path;
            res.set_content(result.dump(), "application/json");
        } else {
            res.status = 500;
            res.set_content(R"({"error":"DB save failed"})", "application/json");
        }
    });

    // GET /api/resumes - list
    svr.Get("/api/resumes", [&jwt_secret](const httplib::Request& req, httplib::Response& res) {
        std::string uid = auth::extractUserId(req, jwt_secret);
        if (uid.empty()) { res.status = 401; res.set_content(R"({"error":"Unauthorized"})", "application/json"); return; }

        Database& db = Database::instance();
        Rows rows = db.query("SELECT id, file_path, upload_time FROM resumes WHERE user_id=" + uid + " ORDER BY upload_time DESC");
        nlohmann::json list = nlohmann::json::array();
        for (auto& r : rows) {
            nlohmann::json item;
            item["id"] = std::stoull(r["id"]);
            item["file_path"] = r["file_path"];
            item["upload_time"] = r["upload_time"];
            // Get file size from disk
            std::ifstream ifs(r["file_path"], std::ios::binary | std::ios::ate);
            item["file_size"] = ifs.is_open() ? static_cast<int64_t>(ifs.tellg()) : 0;
            ifs.close();
            list.push_back(item);
        }
        res.set_content(list.dump(), "application/json");
    });

    // GET /api/resumes/:id/preview - serve original file for browser preview
    svr.Get(R"(/api/resumes/(\d+)/preview)", [&jwt_secret](const httplib::Request& req, httplib::Response& res) {
        // Support token in header OR query param (iframe can't set headers)
        std::string uid = auth::extractUserId(req, jwt_secret);
        if (uid.empty() && req.has_param("token")) {
            uid = jwt::verify(req.get_param_value("token"), jwt_secret);
        }
        if (uid.empty()) { res.status = 401; res.set_content(R"({"error":"Unauthorized"})", "application/json"); return; }

        std::string rid = req.matches[1];
        Database& db = Database::instance();
        Rows rows = db.query("SELECT id, file_path FROM resumes WHERE id=" + rid + " AND user_id=" + uid);
        if (rows.empty()) { res.status = 404; res.set_content(R"({"error":"Not found"})", "application/json"); return; }

        std::string file_path = rows[0]["file_path"];

        // Try multiple paths to find the file (handles working directory changes)
        std::ifstream ifs;
        std::vector<std::string> search_paths = {
            file_path,                                  // relative as-is (uploads/test_resume.docx)
        };
#ifdef _WIN32
        // Also try relative to exe directory
        char exe_buf2[MAX_PATH];
        GetModuleFileNameA(NULL, exe_buf2, MAX_PATH);
        std::string exe_str2(exe_buf2);
        size_t sep2 = exe_str2.find_last_of("\\/");
        if (sep2 != std::string::npos) {
            search_paths.push_back(exe_str2.substr(0, sep2 + 1) + file_path);
        }
#endif
        for (const auto& p : search_paths) {
            ifs.open(p, std::ios::binary | std::ios::ate);
            if (ifs.is_open()) break;
        }
        if (!ifs.is_open()) { res.status = 404; res.set_content(R"({"error":"File not found on disk"})", "application/json"); return; }

        // Read entire file into memory
        std::streamsize size = ifs.tellg();
        ifs.seekg(0, std::ios::beg);
        std::string content(size, '\0');
        ifs.read(&content[0], size);

        // Set content type based on file extension
        std::string mime = "application/octet-stream";
        std::string ext;
        size_t dot = file_path.rfind('.');
        if (dot != std::string::npos) ext = file_path.substr(dot + 1);
        // Lowercase
        for (auto& c : ext) c = static_cast<char>(std::tolower(c));

        if (ext == "pdf") mime = "application/pdf";
        else if (ext == "doc") mime = "application/msword";
        else if (ext == "docx") mime = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
        else if (ext == "txt") mime = "text/plain; charset=utf-8";

        // Extract filename for Content-Disposition
        std::string fname = file_path;
        size_t last_slash = file_path.find_last_of("/\\");
        if (last_slash != std::string::npos) fname = file_path.substr(last_slash + 1);

        res.set_content(content, mime);
        res.set_header("Content-Disposition", "inline; filename=\"" + fname + "\"");
        res.set_header("Cache-Control", "private, max-age=3600");
    });

    // GET /api/resumes/:id/text - extract text content for preview (uses python-docx via subprocess)
    svr.Get(R"(/api/resumes/(\d+)/text)", [&jwt_secret](const httplib::Request& req, httplib::Response& res) {
        std::string uid = auth::extractUserId(req, jwt_secret);
        if (uid.empty() && req.has_param("token")) {
            uid = jwt::verify(req.get_param_value("token"), jwt_secret);
        }
        if (uid.empty()) { res.status = 401; res.set_content(R"({"error":"Unauthorized"})", "application/json"); return; }

        std::string rid = req.matches[1];
        Database& db = Database::instance();
        Rows rows = db.query("SELECT id, file_path FROM resumes WHERE id=" + rid + " AND user_id=" + uid);
        if (rows.empty()) { res.status = 404; res.set_content(R"({"error":"Not found"})", "application/json"); return; }

        std::string file_path = rows[0]["file_path"];
        // Find file (same multi-path search)
        std::string found_path;
        std::vector<std::string> search_paths = { file_path };
#ifdef _WIN32
        char exe_buf[MAX_PATH];
        GetModuleFileNameA(NULL, exe_buf, MAX_PATH);
        std::string exe_str(exe_buf);
        size_t sep = exe_str.find_last_of("\\/");
        if (sep != std::string::npos) search_paths.push_back(exe_str.substr(0, sep + 1) + file_path);
#endif
        for (const auto& p : search_paths) {
            std::ifstream test(p, std::ios::binary);
            if (test.is_open()) { found_path = p; break; }
        }
        if (found_path.empty()) { res.status = 404; res.set_content(R"({"error":"File not found"})", "application/json"); return; }

        // Extract text using python-docx via subprocess
        std::string script = "import sys\n"
            "from docx import Document\n"
            "doc = Document(sys.argv[1])\n"
            "for p in doc.paragraphs:\n"
            "    print(p.text)\n"
            "for table in doc.tables:\n"
            "    for row in table.rows:\n"
            "        cells = [cell.text for cell in row.cells]\n"
            "        print('\\t'.join(cells))\n";

        std::string tmp_script = found_path + ".py";
        {
            std::ofstream ofs(tmp_script);
            ofs << script;
        }

        std::string cmd = "python \"" + tmp_script + "\" \"" + found_path + "\" 2>&1";
        std::string output;
#ifdef _WIN32
        FILE* pipe = _popen(cmd.c_str(), "r");
#else
        FILE* pipe = popen(cmd.c_str(), "r");
#endif
        if (pipe) {
            char buf[4096];
            while (fgets(buf, sizeof(buf), pipe)) output += buf;
#ifdef _WIN32
            _pclose(pipe);
#else
            pclose(pipe);
#endif
        }
        remove(tmp_script.c_str());

        // If python-docx not available, fall back to reading file as raw text
        if (output.empty()) {
            std::ifstream ifs(found_path, std::ios::binary);
            if (ifs.is_open()) {
                // Try reading as UTF-8 text
                output.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
            }
        }

        nlohmann::json result;
        result["content"] = output;
        result["filename"] = rows[0]["file_path"];
        res.set_content(result.dump(), "application/json");
    });

    // DELETE /api/resumes/:id
    svr.Delete(R"(/api/resumes/(\d+))", [&jwt_secret](const httplib::Request& req, httplib::Response& res) {
        std::string uid = auth::extractUserId(req, jwt_secret);
        if (uid.empty()) { res.status = 401; res.set_content(R"({"error":"Unauthorized"})", "application/json"); return; }

        std::string rid = req.matches[1];
        Database& db = Database::instance();
        Rows exist = db.query("SELECT id FROM resumes WHERE id=" + rid + " AND user_id=" + uid);
        if (exist.empty()) { res.status = 404; res.set_content(R"({"error":"Not found"})", "application/json"); return; }
        db.execute("DELETE FROM resumes WHERE id=" + rid + " AND user_id=" + uid);
        res.set_content(R"({"success":true})", "application/json");
    });

    LOG_INFO("Resume", "Routes registered: POST/GET/DELETE /api/resumes");
}

} // namespace resume
