// VerifyCode - In-memory verification code store with expiry
#pragma once

#include <string>
#include <map>
#include <ctime>
#include <cstdlib>
#include <mutex>
#include "logger.h"

class VerifyCode {
public:
    static VerifyCode& instance() { static VerifyCode i; return i; }

    // Generate and store a 6-digit code for an email
    std::string generate(const std::string& email) {
        std::lock_guard<std::mutex> lock(mtx_);
        std::string code;
        for (int i = 0; i < 6; i++) code += std::to_string(rand() % 10);
        codes_[email] = { code, std::time(nullptr) };
        LOG_INFO("VerifyCode", "Generated code for " + email);
        return code;
    }

    // Verify a code (valid for 10 minutes)
    bool verify(const std::string& email, const std::string& code) {
        std::lock_guard<std::mutex> lock(mtx_);
        auto it = codes_.find(email);
        if (it == codes_.end()) return false;

        auto& entry = it->second;
        if (std::time(nullptr) - entry.generated_at > 600) { // 10 min
            codes_.erase(it);
            return false;
        }
        if (entry.code == code) {
            codes_.erase(it);
            return true;
        }
        return false;
    }

private:
    struct Entry {
        std::string code;
        std::time_t generated_at;
    };
    std::map<std::string, Entry> codes_;
    std::mutex mtx_;
};
