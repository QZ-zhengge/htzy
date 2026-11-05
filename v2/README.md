# 红途职引 v2

红色文化主题求职平台，整合岗位、简历、AI 面试和管理功能。

## 代码从哪里读

| 入口 | 负责什么 |
| --- | --- |
| [HTZY_v2.0.cpp](HTZY_v2.0.cpp) | C++ 服务装配入口 |
| [src/server/http_server.h](src/server/http_server.h) | HTTP 层 |
| [src/modules](src/modules) | 认证、岗位、简历、面试和管理模块 |
| [src/db](src/db) | 数据库与迁移 |
| [frontend/src](frontend/src) | Vue 页面和路由 |

## 一条业务链路

从 Vue 页面发起请求，HTTP 层把请求交给业务模块，模块读取数据库或调用 AI 客户端并返回结果。面试展示选一条完整链路，例如岗位查询或简历分析。

## 运行与验证

准备 C++20/CMake、前端 Node 环境和数据库。按 config 示例配置，再分别构建后端和 frontend。部分 CMake 预设使用原机工具链路径，迁移后需调整。整套服务未重新联调。

## 面试可能会问

**重构怎么降低迁移风险？**

按岗位、简历、面试逐条校验接口、状态码和数据结构，把旧系统的关键业务场景当回归用例。

**怎么处理外部模型慢或失败？**

从 AI 客户端调用链看错误边界。面试可讲超时、有限重试和前端错误态，并区分代码现状和改进方案。

## 继续阅读

- [逐文件代码索引](CODE_INDEX.md)
- [作品集展示与面试指南](https://github.com/QZ-zhengge/developer-portfolio/blob/main/docs/INTERVIEW.md)
