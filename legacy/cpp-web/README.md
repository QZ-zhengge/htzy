# 红途职引旧版 C++ 网站

旧版就业网站和 C++ 服务，保留 Visual Studio 工程及静态页面。

## 代码从哪里读

| 入口 | 负责什么 |
| --- | --- |
| [HTZY.sln](HTZY.sln) | VS 解决方案 |
| [HTZY_WebServer/main.cpp](HTZY_WebServer/main.cpp) | C++ 程序入口 |
| [HTZY_WebServer/db_user.cpp](HTZY_WebServer/db_user.cpp) | 用户相关数据库访问 |
| [HTZY_WebServer/page](HTZY_WebServer/page) | 页面与交互脚本 |
| [API.txt](API.txt) | 旧版接口记录 |

## 一条业务链路

按页面请求追踪服务端路由和用户库访问。和 v2 比较时重点看页面、路由、数据库接口各放在哪一层。

## 运行与验证

恢复项目声明的 C++ 依赖与 MySQL 配置后用 Visual Studio 构建，使用自建测试库。历史站点未重新上线验证。

## 面试可能会问

**旧站和 v2 最大结构差别是什么？**

旧站保留静态页面与较集中的 C++ 服务，v2 使用单独 Vue 工程和明确的业务模块。

**接手这套代码先看哪里？**

从启动入口和 API.txt 定位路由，再挑一条用户请求验证 SQL 和返回结构。

## 继续阅读

- [逐文件代码索引](CODE_INDEX.md)
- [作品集展示与面试指南](https://github.com/QZ-zhengge/developer-portfolio/blob/main/docs/INTERVIEW.md)
- [原项目说明与操作细节](README-REFERENCE.md)
