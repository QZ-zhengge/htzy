#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/driver.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <Windows.h>
#include <iostream>
#include <string>
#include <stdexcept>
#include <sstream>
// 引入JSON处理库（需确保项目中已配置nlohmann/json，若未配置可参考文末说明）
#include "json.hpp"

// 简化JSON命名空间
using json = nlohmann::json;

// MySQL 操作工具类
class MySQLHelper {
private:
    sql::Driver* driver;
    sql::Connection* conn;
    std::string host;     // 数据库地址
    std::string user;     // 用户名
    std::string password; // 密码
    std::string database; // 数据库名
    unsigned int port;    // 端口号

    // 初始化连接（私有方法，内部调用）
    void initConnection() {
        try {
            driver = get_driver_instance();
            // 拼接连接字符串：tcp://host:port
            std::string conn_str = "tcp://" + host + ":" + std::to_string(port);
            conn = driver->connect(conn_str, user, password);
            conn->setSchema(database); // 选择数据库
        }
        catch (sql::SQLException& e) {
            throw std::runtime_error("数据库连接失败: " + std::string(e.what()) +
                " [错误码]: " + std::to_string(e.getErrorCode()));
        }
    }

public:
    // 构造函数：初始化数据库连接信息
    MySQLHelper(const std::string& _host, unsigned int _port,
        const std::string& _user, const std::string& _password,
        const std::string& _database)
        : host(_host), port(_port), user(_user), password(_password), database(_database),
        driver(nullptr), conn(nullptr) {
        // 设置控制台编码为UTF8，避免中文乱码
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
        // 初始化连接
        initConnection();
    }

    // 析构函数：释放资源
    ~MySQLHelper() {
        if (conn != nullptr) {
            delete conn;
            conn = nullptr;
        }
    }

    // 功能1：查询数据（返回拼接后的字符串，格式：列名1\t列名2\t...\n值1\t值2\t...\n...）
    // 参数：sql - 查询语句；返回：拼接好的查询结果字符串，失败返回空字符串
    std::string queryData(const std::string& sql) {
        sql::Statement* stmt = nullptr;
        sql::ResultSet* res = nullptr;
        // 用于拼接字符串的流
        std::ostringstream oss;
        try {
            if (conn == nullptr || !conn->isValid()) {
                initConnection(); // 连接失效时重新连接
            }

            stmt = conn->createStatement();
            res = stmt->executeQuery(sql);

            // 获取结果集元数据，拼接列名
            sql::ResultSetMetaData* meta = res->getMetaData();
            int column_count = meta->getColumnCount();
            for (int i = 1; i <= column_count; ++i) {
                oss << meta->getColumnLabel(i);
                if (i < column_count) oss << "\t"; // 列名间用制表符分隔
            }
            oss << "\n"; // 列名行结束

            // 拼接每行数据
            while (res->next()) {
                for (int i = 1; i <= column_count; ++i) {
                    oss << res->getString(i);
                    if (i < column_count) oss << "\t"; // 数据间用制表符分隔
                }
                oss << "\n"; // 每行数据结束
            }

            delete res;
            delete stmt;
            // 返回拼接好的字符串
            return oss.str();
        }
        catch (sql::SQLException& e) {
            std::cerr << "查询失败: " << e.what()
                << " [错误码]: " << e.getErrorCode() << std::endl;
            // 释放资源
            if (res) delete res;
            if (stmt) delete stmt;
            // 失败返回空字符串
            return "";
        }
    }

    // 功能2：将查询返回的字符串转换为JSON格式
    // 参数：query_str - queryData返回的字符串；返回：JSON格式字符串，失败返回空字符串
    // 功能2：将查询返回的字符串转换为JSON格式（修复空列不跳过）
    std::string convertToJson(const std::string& query_str) {
        if (query_str.empty()) {
            std::cerr << "查询结果字符串为空，无法转换JSON" << std::endl;
            return "";
        }

        try {
            json json_result;
            std::vector<std::string> columns;
            std::vector<json> data_list;

            std::istringstream iss(query_str);
            std::string line;
            int line_num = 0;

            while (std::getline(iss, line)) {
                if (line.empty()) continue;

                if (line_num == 0) {
                    // 读取表头（所有列名）
                    std::istringstream col_iss(line);
                    std::string col_name;
                    while (std::getline(col_iss, col_name, '\t')) {
                        columns.push_back(col_name);
                    }
                }
                else {
                    // 读取数据行（修复：不跳过空字段，空字段==""）
                    std::vector<std::string> values;
                    std::string val;
                    std::istringstream line_iss(line);

                    // 逐字段读取，哪怕是空的也保留
                    while (std::getline(line_iss, val, '\t')) {
                        values.push_back(val);
                    }

                    // 组装JSON，保证【每一列都存在】
                    json row;
                    for (int i = 0; i < columns.size(); ++i) {
                        if (i < values.size())
                            row[columns[i]] = values[i];
                        else
                            row[columns[i]] = ""; // 空列补空字符串
                    }
                    data_list.push_back(row);
                }
                line_num++;
            }

            json_result["columns"] = columns;
            json_result["data"] = data_list;
            json_result["total"] = data_list.size();

            return json_result.dump(4);
        }
        catch (const std::exception& e) {
            std::cerr << "JSON转换失败: " << e.what() << std::endl;
            return "";
        }
    }
};

// 测试函数：调用查询和JSON转换
std::string TestMySQL() {
    try {
        // 1. 创建MySQL工具类实例
        MySQLHelper mysql_helper("192.0.2.1", 50306,
            "Invoke User", "qige_060803",
            "htzy_data");

        // 2. 调用查询功能，获取字符串结果
        std::string query_result = mysql_helper.queryData("SELECT * FROM dev_test_001");
        if (query_result.empty()) {
            return "查询结果为空";
        }

        // 3. 转换为JSON格式
        std::string json_result = mysql_helper.convertToJson(query_result);
        return json_result;
    }
    // 替换原catch中的return语句
    catch (const std::exception& e) {
        std::cerr << "Program exception: " << e.what() << std::endl; // 纯英文
        // 用最简洁的方式拼接，避免任何隐形字符
        std::string error_msg = "Exception: ";
        error_msg += e.what();
        return error_msg;
    }
}

// 主函数：调用测试函数并输出结果
int Mysqljson() {
    // 调用TestMySQL获取JSON字符串
    std::string result = TestMySQL();
    // 输出最终的JSON结果
    std::cout << "最终JSON结果：\n" << result << std::endl;

    return 0;
}