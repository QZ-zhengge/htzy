# 红途职引旧站工作副本

旧站在另一工作目录中的快照，用于对照独有页面和接口变化。

## 代码从哪里读

| 入口 | 负责什么 |
| --- | --- |
| [HTZY.sln](HTZY.sln) | 构建入口 |
| [HTZY_WebServer/main.cpp](HTZY_WebServer/main.cpp) | 服务启动 |
| [HTZY_WebServer/page](HTZY_WebServer/page) | 工作副本页面和脚本 |
| [API.txt](API.txt) | 接口记录 |

## 一条业务链路

按页面请求与 C++ 路由对照历史版，重点看 page 下的交互脚本与不同的目录内容。

## 运行与验证

如需复现，独立准备旧版依赖与测试库。该工作副本用于代码对照，主线展示使用 v2。

## 面试可能会问

**为什么同一旧站留两个目录？**

工作副本有不同文件和页面内容，单列可追溯。

**如何决定是否合并？**

对照入口、接口契约、数据依赖和实际差异，再挑有业务价值的文件迁移。

## 继续阅读

- [逐文件代码索引](CODE_INDEX.md)
- [作品集展示与面试指南](https://github.com/QZ-zhengge/developer-portfolio/blob/main/docs/INTERVIEW.md)
- [原项目说明与操作细节](README-REFERENCE.md)
