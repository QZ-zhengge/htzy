// Config - Configuration system
// Reads config/app.conf (JSON), zero hardcoded secrets
#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include "../../third_party/json.hpp"

using json = nlohmann::json;

struct ServerConfig {
    std::string host = "0.0.0.0";
    int port = 8080;
    std::string static_dir = "./frontend/dist";
    std::string jwt_secret = "change_me";
};

struct DatabaseConfig {
    std::string host = "127.0.0.1";
    int port = 3306;
    std::string user = "root";
    std::string password = "";
    std::string dbname = "htzy";
    int pool_size = 8;
    bool auto_init = true;
};

struct DeepseekConfig {
    std::string api_key;
    std::string base_url = "https://api.deepseek.com/v1/chat/completions";
    std::string model = "deepseek-flash";
};

struct SmtpConfig {
    std::string host = "smtp.163.com";
    int port = 465;
    std::string user;
    std::string password;
    std::string from_name = "红途职引";
};

struct AiConfig {
    DeepseekConfig deepseek;
};

struct AutoDeliveryConfig {
    int daily_limit_per_company = 20;
    int daily_limit_per_student = 5;
    int interval_hours = 2;
    int start_hour = 8;
};

struct InterviewConfig {
    int question_count = 20;
    int time_per_question_sec = 60;
    int frame_capture_interval_ms = 1000;
    double weight_professional = 0.4;
    double weight_language = 0.3;
    double weight_expression = 0.3;
    double pass_score = 60.0;
};

struct AppConfig {
    ServerConfig server;
    DatabaseConfig database;
    AiConfig ai;
    SmtpConfig smtp;
    AutoDeliveryConfig auto_delivery;
    InterviewConfig interview;
    std::string log_file;
};

class Config {
public:
    static Config& instance() {
        static Config inst;
        return inst;
    }

    bool load(const std::string& path) {
        std::ifstream f(path);
        if (!f.is_open()) {
            std::cerr << "[Config] Failed to open: " << path << std::endl;
            return false;
        }
        std::ostringstream ss;
        ss << f.rdbuf();
        try {
            json j = json::parse(ss.str());
            if (j.contains("server")) {
                auto& s = j["server"];
                if (s.contains("host")) data_.server.host = s["host"].get<std::string>();
                if (s.contains("port")) data_.server.port = s["port"].get<int>();
                if (s.contains("static_dir")) data_.server.static_dir = s["static_dir"].get<std::string>();
                if (s.contains("jwt_secret")) data_.server.jwt_secret = s["jwt_secret"].get<std::string>();
            }
            if (j.contains("database")) {
                auto& d = j["database"];
                if (d.contains("host")) data_.database.host = d["host"].get<std::string>();
                if (d.contains("port")) data_.database.port = d["port"].get<int>();
                if (d.contains("user")) data_.database.user = d["user"].get<std::string>();
                if (d.contains("password")) data_.database.password = d["password"].get<std::string>();
                if (d.contains("dbname")) data_.database.dbname = d["dbname"].get<std::string>();
                if (d.contains("pool_size")) data_.database.pool_size = d["pool_size"].get<int>();
                if (d.contains("auto_init")) data_.database.auto_init = d["auto_init"].get<bool>();
            }
            if (j.contains("ai")) {
                auto& a = j["ai"];
                if (a.contains("deepseek")) {
                    auto& ds = a["deepseek"];
                    if (ds.contains("api_key")) data_.ai.deepseek.api_key = ds["api_key"].get<std::string>();
                    if (ds.contains("base_url")) data_.ai.deepseek.base_url = ds["base_url"].get<std::string>();
                    if (ds.contains("model")) data_.ai.deepseek.model = ds["model"].get<std::string>();
                }
            }
            if (j.contains("smtp")) {
                auto& s = j["smtp"];
                if (s.contains("host")) data_.smtp.host = s["host"].get<std::string>();
                if (s.contains("port")) data_.smtp.port = s["port"].get<int>();
                if (s.contains("user")) data_.smtp.user = s["user"].get<std::string>();
                if (s.contains("password")) data_.smtp.password = s["password"].get<std::string>();
                if (s.contains("from_name")) data_.smtp.from_name = s["from_name"].get<std::string>();
            }
            if (j.contains("auto_delivery")) {
                auto& ad = j["auto_delivery"];
                if (ad.contains("daily_limit_per_company")) data_.auto_delivery.daily_limit_per_company = ad["daily_limit_per_company"].get<int>();
                if (ad.contains("daily_limit_per_student")) data_.auto_delivery.daily_limit_per_student = ad["daily_limit_per_student"].get<int>();
                if (ad.contains("interval_hours")) data_.auto_delivery.interval_hours = ad["interval_hours"].get<int>();
                if (ad.contains("start_hour")) data_.auto_delivery.start_hour = ad["start_hour"].get<int>();
            }
            if (j.contains("interview")) {
                auto& iv = j["interview"];
                if (iv.contains("question_count")) data_.interview.question_count = iv["question_count"].get<int>();
                if (iv.contains("time_per_question_sec")) data_.interview.time_per_question_sec = iv["time_per_question_sec"].get<int>();
                if (iv.contains("frame_capture_interval_ms")) data_.interview.frame_capture_interval_ms = iv["frame_capture_interval_ms"].get<int>();
                if (iv.contains("pass_score")) data_.interview.pass_score = iv["pass_score"].get<double>();
                if (iv.contains("weights")) {
                    auto& w = iv["weights"];
                    if (w.contains("professional")) data_.interview.weight_professional = w["professional"].get<double>();
                    if (w.contains("language")) data_.interview.weight_language = w["language"].get<double>();
                    if (w.contains("expression")) data_.interview.weight_expression = w["expression"].get<double>();
                }
            }
            if (j.contains("logging")) {
                auto& lg = j["logging"];
                if (lg.contains("file")) data_.log_file = lg["file"].get<std::string>();
            }
            return true;
        } catch (const std::exception& e) {
            std::cerr << "[Config] JSON parse error: " << e.what() << std::endl;
            return false;
        }
    }

    const AppConfig& get() const { return data_; }
    AppConfig& getMutable() { return data_; }

private:
    Config() = default;
    AppConfig data_;
};
