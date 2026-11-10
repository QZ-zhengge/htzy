# 红途职引 v2

红色文化主题求职平台，整合岗位、简历、AI 面试和管理功能。

## 项目结构

| 入口 | 负责什么 |
| --- | --- |
| [HTZY_v2.0.cpp](HTZY_v2.0.cpp) | C++ 服务装配入口 |
| [src/server/http_server.h](src/server/http_server.h) | HTTP 层 |
| [src/modules](src/modules) | 认证、岗位、简历、面试和管理模块 |
| [src/db](src/db) | 数据库与迁移 |
| [frontend/src](frontend/src) | Vue 页面和路由 |

## 运行与验证

准备 C++20/CMake、前端 Node 环境和数据库。按 config 示例配置，再分别构建后端和 frontend。部分 CMake 预设使用原机工具链路径，迁移后需调整。整套服务未重新联调。
