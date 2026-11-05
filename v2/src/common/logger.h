// Logger - Log system with console + file output
// WARNING: Windows.h defines ERROR=0, INFO=0, WARNING=0 as macros
// which conflict with LogLevel enum values. We undef them here.
#pragma once

#ifdef _WIN32
#undef ERROR
#undef WARNING
#undef min
#undef max
#endif

#include <iostream>
#include <string>
#include <ctime>
#include <mutex>
#include <fstream>
#include <sstream>
#include <iomanip>

enum class LogLevel { DEBUG, INFO, WARN, ERROR };

class Logger {
public:
    static Logger& instance() {
        static Logger inst;
        return inst;
    }

    void setLogFile(const std::string& path) {
        std::lock_guard<std::mutex> lock(mtx_);
        if (log_file_.is_open()) log_file_.close();
        if (!path.empty()) {
            log_file_.open(path, std::ios::app);
        }
        log_file_path_ = path;
    }

    void setLevel(LogLevel level) { level_ = level; }

    void log(LogLevel level, const std::string& module, const std::string& msg) {
        if (level < level_) return;
        std::lock_guard<std::mutex> lock(mtx_);
        std::string line = formatLine(level, module, msg);
        std::cout << line << std::endl;
        if (log_file_.is_open()) {
            log_file_ << line << std::endl;
            log_file_.flush();
        }
    }

private:
    Logger() : level_(LogLevel::DEBUG) {}
    ~Logger() { if (log_file_.is_open()) log_file_.close(); }

    LogLevel level_;
    std::string log_file_path_;
    std::ofstream log_file_;
    std::mutex mtx_;

    std::string levelStr(LogLevel level) {
        switch (level) {
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO:  return "INFO ";
            case LogLevel::WARN:  return "WARN ";
            case LogLevel::ERROR: return "ERROR";
            default: return "?????";
        }
    }

    std::string now() {
        auto t = std::time(nullptr);
        std::tm tm_buf;
#ifdef _WIN32
        localtime_s(&tm_buf, &t);
#else
        localtime_r(&t, &tm_buf);
#endif
        std::ostringstream oss;
        oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

    std::string formatLine(LogLevel level, const std::string& module, const std::string& msg) {
        std::ostringstream oss;
        oss << "[" << now() << "] [" << levelStr(level) << "] [" << module << "] " << msg;
        return oss.str();
    }
};

#define LOG_DEBUG(module, msg) Logger::instance().log(LogLevel::DEBUG, module, msg)
#define LOG_INFO(module, msg)  Logger::instance().log(LogLevel::INFO,  module, msg)
#define LOG_WARN(module, msg)  Logger::instance().log(LogLevel::WARN,  module, msg)
#define LOG_ERROR(module, msg) Logger::instance().log(LogLevel::ERROR, module, msg)
