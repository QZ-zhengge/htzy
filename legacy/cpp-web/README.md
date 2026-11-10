# 红途职引旧版 C++ 网站

旧版就业网站和 C++ 服务，保留 Visual Studio 工程及静态页面。

## 项目结构

| 入口 | 负责什么 |
| --- | --- |
| [HTZY.sln](HTZY.sln) | VS 解决方案 |
| [HTZY_WebServer/main.cpp](HTZY_WebServer/main.cpp) | C++ 程序入口 |
| [HTZY_WebServer/db_user.cpp](HTZY_WebServer/db_user.cpp) | 用户相关数据库访问 |
| [HTZY_WebServer/page](HTZY_WebServer/page) | 页面与交互脚本 |
| [API.txt](API.txt) | 旧版接口记录 |

## 运行与验证

恢复项目声明的 C++ 依赖与 MySQL 配置后用 Visual Studio 构建，使用自建测试库。历史站点未重新上线验证。

## 继续阅读

- [原项目说明与操作细节](README-REFERENCE.md)
