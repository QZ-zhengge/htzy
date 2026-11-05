#include <iostream>
#include <vector>
#include <map>
// 引入依赖库（确保两个头文件在同一目录）
#include "httplib.h"
#include "json.hpp"

// 别名简化代码
using json = nlohmann::json;
using namespace httplib;

// 1. 定义模拟数据（对应前端JS需要的结构，实际项目可替换为数据库查询）
// 行业-地区映射（对应前端 industryRegions）
std::map<std::u8string, std::vector<std::u8string>> g_industry_regions = {
    {u8"all", {u8"全国", u8"北京", u8"上海", u8"广州", u8"深圳"}},
    {u8"互联网", {u8"北京", u8"上海", u8"广州", u8"深圳", u8"杭州"}},
    {u8"金融", {u8"北京", u8"上海", u8"广州", u8"深圳", u8"香港"}},
    {u8"制造业", {u8"上海", u8"苏州", u8"东莞", u8"深圳", u8"天津"}},
    {u8"教育", {u8"北京", u8"上海", u8"广州", u8"深圳", u8"成都"}}
};

// 职位数据结构（对应前端 job 对象字段）
struct Job {
    std::u8string title;       // 职位名称
    std::u8string company;     // 公司名称
    std::u8string location;    // 工作地点
    std::u8string experience;  // 工作经验要求
    std::u8string education;   // 学历要求
    std::vector<std::u8string> tags;  // 职位标签
    std::u8string salary;      // 薪资
    std::u8string industry;    // 所属行业
};

// 模拟职位数据（对应前端 jobsData）
std::vector<Job> g_jobs_data = {
    {u8"后端开发工程师", u8"某互联网科技有限公司", u8"北京", u8"3-5年", u8"本科", {u8"C++", u8"后端", u8"分布式"}, u8"25k-40k", u8"互联网"},
    {u8"前端开发工程师", u8"某网络科技有限公司", u8"上海", u8"1-3年", u8"本科", {u8"JavaScript", u8"React", u8"Vue"}, u8"18k-30k", u8"互联网"},
    {u8"金融产品经理", u8"某国有银行总行", u8"北京", u8"5-8年", u8"硕士", {u8"金融", u8"产品", u8"风控"}, u8"30k-50k", u8"金融"},
    {u8"机械工程师", u8"某精密制造有限公司", u8"苏州", u8"3-5年", u8"本科", {u8"机械设计", u8"CAD", u8"智能制造"}, u8"15k-25k", u8"制造业"},
    {u8"高中语文教师", u8"某知名教育机构", u8"成都", u8"2-4年", u8"本科", {u8"语文", u8"高考", u8"教研"}, u8"12k-20k", u8"教育"}
};


void StartHttpServer()
{
    // 初始化HTTP服务器，监听 localhost:8080
    Server svr;

    svr.set_mount_point("/page", "Z:/HTZY/HTZY_Page/x64/Release/page");

    // 2. 实现 /api/industries 接口（返回行业-地区映射）
    svr.Get("/api/industries", [&](const Request& req, Response& res) {
        // 配置跨域响应头（前端直接请求会出现跨域限制，必须添加）
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");

        // 将 std::map 转换为 JSON 格式
        res.set_header("Content-Type", "application/json; charset=utf-8");
        json industry_json = g_industry_regions;

        // 设置响应内容类型为 JSON，返回序列化后的 JSON 字符串
        std::string json_str = industry_json.dump(-1, ' ', false, json::error_handler_t::replace);
        res.set_content(json_str, "utf-8");
        });

    // 3. 实现 /api/jobs 接口
    svr.Get("/api/jobs", [&](const Request& req, Response& res)
        {
            // 配置跨域响应头
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
            res.set_header("Access-Control-Allow-Headers", "Content-Type");

            // 转换 std::vector<Job> 为 JSON 格式
            //res.set_header("Content-Type", "application/json; charset=utf-8");
            //json jobs_json = json::array();
            json jobs_list = json::array();
            for (const auto& job : g_jobs_data)
            {
                json job_json;
                job_json["title"] = job.title;
                job_json["company"] = job.company;
                job_json["location"] = job.location;
                job_json["experience"] = job.experience;
                job_json["education"] = job.education;
                job_json["tags"] = job.tags;
                job_json["salary"] = job.salary;
                job_json["industry"] = job.industry;
                jobs_list.push_back(job_json);
            }

            // 设置响应内容类型为 JSON，返回序列化后的 JSON 字符串
            std::string json_str = jobs_list.dump(-1, ' ', false, json::error_handler_t::replace);
            res.set_content(json_str, "application/json; charset=utf-8");
        });

    // 启动服务器
    std::cout << "服务器已启动，监听 http://localhost:8080" << std::endl;
    std::cout << "按 Ctrl+C 停止服务器" << std::endl;
    svr.listen("0.0.0.0", 8080);
}