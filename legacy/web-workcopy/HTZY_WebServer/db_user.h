#pragma once
#include <string>
#include <vector>
#include "json.hpp"

using json = nlohmann::json;

// 初始化数据库（创建所有表）
bool initUserDatabase();

// 用户注册
bool registerUser(const std::string& username, const std::string& password,
    const std::string& email, const std::string& phone, int& userId);

// 用户登录（返回 userId，-1 表示失败）
int loginUser(const std::string& username, const std::string& password);

// 获取用户基本信息
json getUserProfile(int userId);

// 更新用户基本信息
bool updateUserProfile(int userId, const json& data);

// 技能操作
std::vector<std::string> getSkills(int userId);
bool addSkill(int userId, const std::string& skill);
bool removeSkill(int userId, const std::string& skill);

// 简历操作
std::string getResume(int userId);
bool updateResume(int userId, const std::string& content);

// 面试历史（分页+搜索）
json getHistory(int userId, int page, int pageSize, const std::string& keyword);

// 分析数据（雷达图、趋势、优势、建议等）
json getAnalysisData(int userId, const std::string& timeRange);