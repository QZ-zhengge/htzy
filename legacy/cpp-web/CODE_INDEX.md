# 红途职引旧版 C++ 网站 · 代码索引

从 [项目 README](README.md) 了解主流程。下表把当前快照中的源码、页面、脚本与主要配置按目录列出。职责根据代码声明与目录用途概括。

| 文件 | 职责或阅读线索 |
| --- | --- |
| [HTZY_WebServer/HttpServer.cpp](HTZY_WebServer/HttpServer.cpp) | 该目录下的业务实现或工具代码。可从 `Job` 开始 |
| [HTZY_WebServer/dashborad_dbtest.cpp](HTZY_WebServer/dashborad_dbtest.cpp) | 该目录下的业务实现或工具代码。可从 `MySQLHelper` 开始 |
| [HTZY_WebServer/db_user.cpp](HTZY_WebServer/db_user.cpp) | 用户相关数据库访问 |
| [HTZY_WebServer/db_user.h](HTZY_WebServer/db_user.h) | 该目录下的业务实现或工具代码 |
| [HTZY_WebServer/index_dbtest.cpp](HTZY_WebServer/index_dbtest.cpp) | 该目录下的业务实现或工具代码。可从 `MySQLHelper` 开始 |
| [HTZY_WebServer/init_db.sql](HTZY_WebServer/init_db.sql) | 处理存储连接、表结构或数据迁移 |
| [HTZY_WebServer/main.cpp](HTZY_WebServer/main.cpp) | C++ 程序入口 |
| [HTZY_WebServer/page/Final_Version.js](HTZY_WebServer/page/Final_Version.js) | 实现页面、组件或用户交互。可从 `setActiveNav` 、 `destroyAutoPlay` 、 `showHome` 、 `showRedGuide` 开始 |
| [HTZY_WebServer/page/dashborad.html](HTZY_WebServer/page/dashborad.html) | 实现页面、组件或用户交互 |
| [HTZY_WebServer/page/index.html](HTZY_WebServer/page/index.html) | 实现页面、组件或用户交互 |
| [HTZY_WebServer/page/login.html](HTZY_WebServer/page/login.html) | 实现页面、组件或用户交互 |
| [HTZY_WebServer/page/profile.css](HTZY_WebServer/page/profile.css) | 实现页面、组件或用户交互 |
| [HTZY_WebServer/page/register.html](HTZY_WebServer/page/register.html) | 实现页面、组件或用户交互 |
| [HTZY_WebServer/page/styles.css](HTZY_WebServer/page/styles.css) | 实现页面、组件或用户交互 |
| [HTZY_WebServer/page/user-center.js](HTZY_WebServer/page/user-center.js) | 实现页面、组件或用户交互。可从 `apiRequest` 、 `fetchUserProfile` 、 `fetchHistoryData` 、 `getDefaultHistoryData` 开始 |
| [HTZY_WebServer/red-guide.html](HTZY_WebServer/red-guide.html) | 定义界面结构与交互 |
| [HTZY_WebServer/redGuide.js](HTZY_WebServer/redGuide.js) | 该目录下的业务实现或工具代码。可从 `escapeHtml` 、 `renderCarousel` 、 `renderStories` 、 `updateCarouselPosition` 开始 |
| [HTZY_WebServer/server.cpp](HTZY_WebServer/server.cpp) | 应用或子模块入口 |

## 第三方、框架及设计器文件

这些文件主要由上游依赖、构建框架或设计器提供。面试讲解可从它们如何支撑项目入手。

- [HTZY_WebServer/crow.h](HTZY_WebServer/crow.h)
- [HTZY_WebServer/crow/TinySHA1.hpp](HTZY_WebServer/crow/TinySHA1.hpp)
- [HTZY_WebServer/crow/app.h](HTZY_WebServer/crow/app.h)
- [HTZY_WebServer/crow/ci_map.h](HTZY_WebServer/crow/ci_map.h)
- [HTZY_WebServer/crow/common.h](HTZY_WebServer/crow/common.h)
- [HTZY_WebServer/crow/compression.h](HTZY_WebServer/crow/compression.h)
- [HTZY_WebServer/crow/exceptions.h](HTZY_WebServer/crow/exceptions.h)
- [HTZY_WebServer/crow/http_connection.h](HTZY_WebServer/crow/http_connection.h)
- [HTZY_WebServer/crow/http_parser_merged.h](HTZY_WebServer/crow/http_parser_merged.h)
- [HTZY_WebServer/crow/http_request.h](HTZY_WebServer/crow/http_request.h)
- [HTZY_WebServer/crow/http_response.h](HTZY_WebServer/crow/http_response.h)
- [HTZY_WebServer/crow/http_server.h](HTZY_WebServer/crow/http_server.h)
- [HTZY_WebServer/crow/json.h](HTZY_WebServer/crow/json.h)
- [HTZY_WebServer/crow/logging.h](HTZY_WebServer/crow/logging.h)
- [HTZY_WebServer/crow/middleware.h](HTZY_WebServer/crow/middleware.h)
- [HTZY_WebServer/crow/middleware_context.h](HTZY_WebServer/crow/middleware_context.h)
- [HTZY_WebServer/crow/middlewares/cookie_parser.h](HTZY_WebServer/crow/middlewares/cookie_parser.h)
- [HTZY_WebServer/crow/middlewares/cors.h](HTZY_WebServer/crow/middlewares/cors.h)
- [HTZY_WebServer/crow/middlewares/session.h](HTZY_WebServer/crow/middlewares/session.h)
- [HTZY_WebServer/crow/middlewares/utf-8.h](HTZY_WebServer/crow/middlewares/utf-8.h)
- [HTZY_WebServer/crow/mime_types.h](HTZY_WebServer/crow/mime_types.h)
- [HTZY_WebServer/crow/multipart.h](HTZY_WebServer/crow/multipart.h)
- [HTZY_WebServer/crow/multipart_view.h](HTZY_WebServer/crow/multipart_view.h)
- [HTZY_WebServer/crow/mustache.h](HTZY_WebServer/crow/mustache.h)
- [HTZY_WebServer/crow/parser.h](HTZY_WebServer/crow/parser.h)
- [HTZY_WebServer/crow/query_string.h](HTZY_WebServer/crow/query_string.h)
- [HTZY_WebServer/crow/returnable.h](HTZY_WebServer/crow/returnable.h)
- [HTZY_WebServer/crow/routing.h](HTZY_WebServer/crow/routing.h)
- [HTZY_WebServer/crow/settings.h](HTZY_WebServer/crow/settings.h)
- [HTZY_WebServer/crow/socket_acceptors.h](HTZY_WebServer/crow/socket_acceptors.h)
- [HTZY_WebServer/crow/socket_adaptors.h](HTZY_WebServer/crow/socket_adaptors.h)
- [HTZY_WebServer/crow/task_timer.h](HTZY_WebServer/crow/task_timer.h)
- [HTZY_WebServer/crow/utility.h](HTZY_WebServer/crow/utility.h)
- [HTZY_WebServer/crow/version.h](HTZY_WebServer/crow/version.h)
- [HTZY_WebServer/crow/websocket.h](HTZY_WebServer/crow/websocket.h)
- [HTZY_WebServer/httplib.h](HTZY_WebServer/httplib.h)
- [HTZY_WebServer/json.hpp](HTZY_WebServer/json.hpp)

本页列出 18 个可阅读的项目文件，以及 37 个第三方或生成文件。
