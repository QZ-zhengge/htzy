// DeepSeek AI Client - calls DeepSeek REST API
// Uses WinHTTP for HTTPS (no OpenSSL dependency)
#pragma once

#ifdef _WIN32

#include <string>
#include <sstream>
#include <vector>
#include <Windows.h>
#include <winhttp.h>
#include "../../common/config.h"
#include "../../common/logger.h"
#include "../../third_party/json.hpp"

#pragma comment(lib, "winhttp.lib")

using json = nlohmann::json;

class DeepseekClient {
public:
    static DeepseekClient& instance() {
        static DeepseekClient inst;
        return inst;
    }

    json chat(const std::string& prompt, int max_tokens = 3072, double temperature = 0.1) {
        const auto& cfg = Config::instance().get().ai.deepseek;
        if (cfg.api_key.empty()) {
            LOG_ERROR("DeepSeek", "API key not configured");
            return nullptr;
        }

        std::string forced = "You must output ONLY valid JSON, no explanation, no prefix, no suffix.\n"
                             "Follow the requested JSON format exactly.\n\n" + prompt;

        json body;
        body["model"] = cfg.model;
        body["messages"] = json::array({{{"role", "user"}, {"content", forced}}});
        body["temperature"] = temperature;
        body["max_tokens"] = max_tokens;
        body["stream"] = false;

        std::string req_body = body.dump();
        std::string response = httpsPost("api.deepseek.com", L"/v1/chat/completions",
                                          "Bearer " + cfg.api_key, req_body);

        if (response.empty()) {
            LOG_ERROR("DeepSeek", "Empty response");
            return nullptr;
        }

        try {
            json resp = json::parse(response);
            std::string content = resp["choices"][0]["message"]["content"].get<std::string>();

            // Clean markdown code blocks
            if (content.size() > 3 && content.substr(0, 3) == "```") {
                content = content.substr(3);
                if (content.size() > 4 && content.substr(0, 4) == "json") content = content.substr(4);
                auto end = content.rfind("```");
                if (end != std::string::npos) content = content.substr(0, end);
            }
            size_t s = content.find_first_not_of(" \t\n\r");
            size_t e = content.find_last_not_of(" \t\n\r");
            if (s != std::string::npos) content = content.substr(s, e - s + 1);

            LOG_INFO("DeepSeek", "Response OK (" + std::to_string(content.size()) + " chars)");
            return json::parse(content);
        } catch (const std::exception& ex) {
            LOG_ERROR("DeepSeek", std::string("Parse error: ") + ex.what());
            return nullptr;
        }
    }

    json analyzeResume(const std::string& resume_content, const std::string& job_position) {
        return chat("Analyze this resume for " + job_position + ". Return JSON:\n"
            "{\"basic_info\":{\"name\":\"\",\"contact\":\"\"},\"experience_summary\":\"\","
            "\"education_summary\":\"\",\"skills\":[],\"project_summary\":\"\","
            "\"match_points\":[],\"interview_focus\":[]}\n\nResume:\n" + resume_content);
    }

    json generateQuestions(const json& resume_info, const std::string& job_position, int count = 20) {
        return chat("Generate " + std::to_string(count) + " interview questions for " + job_position +
            ". Return JSON:{\"questions\":[{\"text\":\"...\",\"type\":\"technical/behavioral/situational\",\"difficulty\":1-5}]}\n"
            "Resume:" + resume_info.dump() + "\nPosition:" + job_position, 4096);
    }

    json analyzeAnswer(const std::string& question, const std::string& answer, const std::string& job_position) {
        return chat("Score this answer (0-100). Return JSON:\n"
            "{\"professional_score\":0-100,\"language_score\":0-100,\"logic_score\":0-100,"
            "\"adaptability_score\":0-100,\"comment\":\"brief\",\"suggestion\":\"advice\"}\n"
            "Question:" + question + "\nAnswer:" + answer + "\nPosition:" + job_position);
    }

    json analyzeVoice(const std::string& text) {
        return chat("Analyze speech and score 4 dimensions (0-100). Return JSON:\n"
            "{\"mandarin\":0-100,\"stability\":0-100,\"clarity\":0-100,\"organization\":0-100}\n"
            "Content:" + text, 1024);
    }

    json generateReport(const json& interview_data, const json& facial,
                        const json& voice, const json& professional) {
        return chat("Generate interview report. Return JSON:\n"
            "{\"summary\":\"\",\"ability_assessment\":{},\"strengths\":[],"
            "\"weaknesses\":[],\"improvements\":[],\"overall_evaluation\":\"\"}\n"
            "Data:" + interview_data.dump() + "\nFacial:" + facial.dump() +
            "\nVoice:" + voice.dump() + "\nProfessional:" + professional.dump(), 4096);
    }

private:
    std::string httpsPost(const std::string& host, const std::wstring& path,
                          const std::string& auth_header, const std::string& body) {
        // Convert host to wide string
        std::wstring w_host(host.begin(), host.end());

        HINTERNET hSession = WinHttpOpen(L"HTZY/2.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                          WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
        if (!hSession) { LOG_ERROR("DeepSeek", "WinHttpOpen failed"); return ""; }

        HINTERNET hConnect = WinHttpConnect(hSession, w_host.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
        if (!hConnect) { WinHttpCloseHandle(hSession); LOG_ERROR("DeepSeek", "WinHttpConnect failed"); return ""; }

        HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", path.c_str(),
                                                  NULL, WINHTTP_NO_REFERER,
                                                  WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
        if (!hRequest) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); LOG_ERROR("DeepSeek", "WinHttpOpenRequest failed"); return ""; }

        // SSL: only ignore cert date mismatch (for dev environments with wrong system time)
        // Production: remove this entirely
        DWORD sec_flags = SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;
        WinHttpSetOption(hRequest, WINHTTP_OPTION_SECURITY_FLAGS, &sec_flags, sizeof(sec_flags));

        // Headers
        std::string headers = "Content-Type: application/json\r\nAuthorization: " + auth_header;
        std::wstring w_headers(headers.begin(), headers.end());

        BOOL result = WinHttpSendRequest(hRequest, w_headers.c_str(), (DWORD)w_headers.size(),
                                          (LPVOID)body.c_str(), (DWORD)body.size(),
                                          (DWORD)body.size(), 0);

        if (!result) {
            WinHttpCloseHandle(hRequest); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession);
            LOG_ERROR("DeepSeek", "WinHttpSendRequest failed: " + std::to_string(GetLastError()));
            return "";
        }

        result = WinHttpReceiveResponse(hRequest, NULL);
        if (!result) {
            WinHttpCloseHandle(hRequest); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession);
            LOG_ERROR("DeepSeek", "WinHttpReceiveResponse failed");
            return "";
        }

        // Read response
        std::string response;
        DWORD bytes_available = 0;
        while (WinHttpQueryDataAvailable(hRequest, &bytes_available) && bytes_available > 0) {
            std::vector<char> buf(bytes_available);
            DWORD bytes_read = 0;
            WinHttpReadData(hRequest, buf.data(), bytes_available, &bytes_read);
            response.append(buf.data(), bytes_read);
        }

        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);

        LOG_INFO("DeepSeek", "HTTP POST " + host + " -> " + std::to_string(response.size()) + " bytes");
        return response;
    }
};

#else
// Linux/macOS: DeepSeek client using system curl
#include <cstdio>

using json = nlohmann::json;

class DeepseekClient {
public:
    static DeepseekClient& instance() { static DeepseekClient i; return i; }

    json chat(const std::string& prompt, int max_tokens = 3072, double temperature = 0.1) {
        const auto& cfg = Config::instance().get().ai.deepseek;
        if (cfg.api_key.empty()) {
            LOG_ERROR("DeepSeek", "API key not configured");
            return nullptr;
        }

        std::string forced = "You must output ONLY valid JSON, no explanation, no prefix, no suffix.\n"
                             "Follow the requested JSON format exactly.\n\n" + prompt;

        json body;
        body["model"] = cfg.model;
        body["messages"] = json::array({{{"role", "user"}, {"content", forced}}});
        body["temperature"] = temperature;
        body["max_tokens"] = max_tokens;
        body["stream"] = false;

        std::string req_body = body.dump();

        // Write request body to temp file, call curl, read response
        std::string tmp_req = "/tmp/htzy_deepseek_req.json";
        std::string tmp_resp = "/tmp/htzy_deepseek_resp.json";

        // Write request
        {
            std::ofstream ofs(tmp_req);
            ofs << req_body;
        }

        // Build curl command
        std::string cmd = "curl -s -X POST https://api.deepseek.com/v1/chat/completions "
            "-H \"Content-Type: application/json\" "
            "-H \"Authorization: Bearer " + cfg.api_key + "\" "
            "-d @" + tmp_req + " -o " + tmp_resp + " --max-time 30 2>/dev/null";

        LOG_INFO("DeepSeek", "Calling API via curl...");
        int ret = system(cmd.c_str());
        if (ret != 0) {
            LOG_ERROR("DeepSeek", "curl failed with code " + std::to_string(ret));
            return nullptr;
        }

        // Read response
        std::ifstream ifs(tmp_resp);
        if (!ifs.is_open()) {
            LOG_ERROR("DeepSeek", "Cannot read response file");
            return nullptr;
        }
        std::string response((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

        // Cleanup temp files
        std::remove(tmp_req.c_str());
        std::remove(tmp_resp.c_str());

        if (response.empty()) {
            LOG_ERROR("DeepSeek", "Empty response");
            return nullptr;
        }

        try {
            json resp = json::parse(response);
            if (resp.contains("error")) {
                LOG_ERROR("DeepSeek", "API error: " + resp["error"].dump());
                return nullptr;
            }
            std::string content = resp["choices"][0]["message"]["content"].get<std::string>();

            // Clean markdown code blocks
            if (content.size() > 3 && content.substr(0, 3) == "```") {
                content = content.substr(3);
                if (content.size() > 4 && content.substr(0, 4) == "json") content = content.substr(4);
                auto end = content.rfind("```");
                if (end != std::string::npos) content = content.substr(0, end);
            }
            size_t s = content.find_first_not_of(" \t\n\r");
            size_t e = content.find_last_not_of(" \t\n\r");
            if (s != std::string::npos) content = content.substr(s, e - s + 1);

            LOG_INFO("DeepSeek", "Response OK (" + std::to_string(content.size()) + " chars)");
            return json::parse(content);
        } catch (const std::exception& ex) {
            LOG_ERROR("DeepSeek", std::string("Parse error: ") + ex.what());
            return nullptr;
        }
    }

    json analyzeResume(const std::string& resume_content, const std::string& job_position) {
        return chat("Analyze this resume for " + job_position + ". Return JSON:\n"
            "{\"basic_info\":{\"name\":\"\",\"contact\":\"\"},\"experience_summary\":\"\","
            "\"education_summary\":\"\",\"skills\":[],\"project_summary\":\"\","
            "\"match_points\":[],\"interview_focus\":[]}\n\nResume:\n" + resume_content);
    }

    json generateQuestions(const json& resume_info, const std::string& job_position, int count = 20) {
        return chat("Generate " + std::to_string(count) + " interview questions for " + job_position +
            ". Return JSON:{\"questions\":[{\"text\":\"...\",\"type\":\"technical/behavioral/situational\",\"difficulty\":1-5}]}\n"
            "Resume:" + resume_info.dump() + "\nPosition:" + job_position, 4096);
    }

    json analyzeAnswer(const std::string& question, const std::string& answer, const std::string& job_position) {
        return chat("Score this answer (0-100). Return JSON:\n"
            "{\"professional_score\":0-100,\"language_score\":0-100,\"logic_score\":0-100,"
            "\"adaptability_score\":0-100,\"comment\":\"brief\",\"suggestion\":\"advice\"}\n"
            "Question:" + question + "\nAnswer:" + answer + "\nPosition:" + job_position);
    }

    json analyzeVoice(const std::string& text) {
        return chat("Analyze speech and score 4 dimensions (0-100). Return JSON:\n"
            "{\"mandarin\":0-100,\"stability\":0-100,\"clarity\":0-100,\"organization\":0-100}\n"
            "Content:" + text, 1024);
    }

    json generateReport(const json& interview_data, const json& facial,
                        const json& voice, const json& professional) {
        return chat("Generate interview report. Return JSON:\n"
            "{\"summary\":\"\",\"ability_assessment\":{},\"strengths\":[],"
            "\"weaknesses\":[],\"improvements\":[],\"overall_evaluation\":\"\"}\n"
            "Data:" + interview_data.dump() + "\nFacial:" + facial.dump() +
            "\nVoice:" + voice.dump() + "\nProfessional:" + professional.dump(), 4096);
    }
};
#endif
