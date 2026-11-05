# 红途职引 v2 · 代码索引

从 [项目 README](README.md) 了解主流程。下表把当前快照中的源码、页面、脚本与主要配置按目录列出。职责根据代码声明与目录用途概括。

| 文件 | 职责或阅读线索 |
| --- | --- |
| [CMakeLists.txt](CMakeLists.txt) | 该目录下的业务实现或工具代码 |
| [CMakePresets.json](CMakePresets.json) | 提供工程配置、界面资源或结构化数据 |
| [HTZY_v2.0.cpp](HTZY_v2.0.cpp) | C++ 服务装配入口 |
| [HTZY_v2.0.h](HTZY_v2.0.h) | 该目录下的业务实现或工具代码 |
| [frontend/index.html](frontend/index.html) | 应用或子模块入口 |
| [frontend/package.json](frontend/package.json) | 提供工程配置、界面资源或结构化数据 |
| [frontend/src/App.vue](frontend/src/App.vue) | 应用或子模块入口 |
| [frontend/src/api/index.js](frontend/src/api/index.js) | 封装接口调用、连接或数据解析 |
| [frontend/src/components/Navbar.vue](frontend/src/components/Navbar.vue) | 实现页面、组件或用户交互。可从 `toggleDropdown` 、 `toggleMobileMenu` 、 `handleLogout` 开始 |
| [frontend/src/main.js](frontend/src/main.js) | 应用或子模块入口 |
| [frontend/src/router/index.js](frontend/src/router/index.js) | 定义 HTTP 请求入口、参数或响应 |
| [frontend/src/stores/user.js](frontend/src/stores/user.js) | 管理本地状态、偏好或持久化 |
| [frontend/src/style.css](frontend/src/style.css) | 定义界面样式与布局 |
| [frontend/src/views/AutoDelivery.vue](frontend/src/views/AutoDelivery.vue) | 实现页面、组件或用户交互。可从 `saveConfig` 、 `formatDate` 开始 |
| [frontend/src/views/Dashboard.vue](frontend/src/views/Dashboard.vue) | 实现页面、组件或用户交互。可从 `loadData` 、 `handleCreateInterview` 、 `formatDate` 、 `getScoreClass` 开始 |
| [frontend/src/views/Home.vue](frontend/src/views/Home.vue) | 实现页面、组件或用户交互 |
| [frontend/src/views/Interview.vue](frontend/src/views/Interview.vue) | 实现页面、组件或用户交互。可从 `getRandomTip` 、 `loadInterview` 、 `startInterview` 、 `initCamera` 开始 |
| [frontend/src/views/Jobs.vue](frontend/src/views/Jobs.vue) | 实现页面、组件或用户交互。可从 `loadJobs` 、 `handleSearch` 、 `handleFilter` 、 `changePage` 开始 |
| [frontend/src/views/Login.vue](frontend/src/views/Login.vue) | 实现页面、组件或用户交互。可从 `handleLogin` 开始 |
| [frontend/src/views/RedGuide.vue](frontend/src/views/RedGuide.vue) | 实现页面、组件或用户交互。可从 `getCategoryLabel` 开始 |
| [frontend/src/views/RedGuideDetail.vue](frontend/src/views/RedGuideDetail.vue) | 实现页面、组件或用户交互。可从 `getCategoryLabel` 开始 |
| [frontend/src/views/Register.vue](frontend/src/views/Register.vue) | 实现页面、组件或用户交互。可从 `handleSendCode` 、 `handleRegister` 开始 |
| [frontend/src/views/Results.vue](frontend/src/views/Results.vue) | 实现页面、组件或用户交互。可从 `abilityLabel` 、 `loadResults` 、 `getScoreDescription` 开始 |
| [frontend/src/views/Resume.vue](frontend/src/views/Resume.vue) | 实现页面、组件或用户交互。可从 `loadResumes` 、 `handleFileSelect` 、 `triggerFileInput` 、 `handleDrop` 开始 |
| [frontend/src/views/VerifyPending.vue](frontend/src/views/VerifyPending.vue) | 实现页面、组件或用户交互。可从 `logout` 开始 |
| [frontend/src/views/admin/AdminDashboard.vue](frontend/src/views/admin/AdminDashboard.vue) | 实现页面、组件或用户交互。可从 `handleLogout` 开始 |
| [frontend/src/views/admin/AdminLogin.vue](frontend/src/views/admin/AdminLogin.vue) | 实现页面、组件或用户交互。可从 `handleLogin` 开始 |
| [frontend/src/views/admin/AdminScores.vue](frontend/src/views/admin/AdminScores.vue) | 实现页面、组件或用户交互。可从 `exportCSV` 、 `formatDate` 、 `statusText` 、 `handleLogout` 开始 |
| [frontend/src/views/admin/AdminStudents.vue](frontend/src/views/admin/AdminStudents.vue) | 实现页面、组件或用户交互。可从 `loadRoster` 、 `loadRegistered` 、 `handleJsonUpload` 、 `handleFileSelect` 开始 |
| [frontend/src/views/admin/AdminSuper.vue](frontend/src/views/admin/AdminSuper.vue) | 实现页面、组件或用户交互。可从 `handleAdd` 、 `handleLogout` 开始 |
| [frontend/vite.config.js](frontend/vite.config.js) | 配置构建、应用启动或环境参数 |
| [src/ai/deepseek_client.h](src/ai/deepseek_client.h) | 该目录下的业务实现或工具代码。可从 `DeepseekClient` 开始 |
| [src/common/config.h](src/common/config.h) | 配置构建、应用启动或环境参数。可从 `ServerConfig` 、 `DatabaseConfig` 、 `DeepseekConfig` 、 `SmtpConfig` 开始 |
| [src/common/jwt.h](src/common/jwt.h) | 该目录下的业务实现或工具代码 |
| [src/common/logger.h](src/common/logger.h) | 该目录下的业务实现或工具代码。可从 `class` 、 `Logger` 开始 |
| [src/common/mailer.h](src/common/mailer.h) | 该目录下的业务实现或工具代码。可从 `Mailer` 开始 |
| [src/common/verify_code.h](src/common/verify_code.h) | 该目录下的业务实现或工具代码。可从 `VerifyCode` 、 `Entry` 开始 |
| [src/db/database.h](src/db/database.h) | 处理存储连接、表结构或数据迁移。可从 `Database` 开始 |
| [src/db/schema_migrator.h](src/db/schema_migrator.h) | 处理存储连接、表结构或数据迁移。可从 `SchemaMigrator` 、 `Story` 开始 |
| [src/modules/admin/admin_handler.h](src/modules/admin/admin_handler.h) | 定义 HTTP 请求入口、参数或响应 |
| [src/modules/analysis/analysis_handler.h](src/modules/analysis/analysis_handler.h) | 定义 HTTP 请求入口、参数或响应 |
| [src/modules/auth/auth_handler.h](src/modules/auth/auth_handler.h) | 定义 HTTP 请求入口、参数或响应。可从 `HttpServer` 开始 |
| [src/modules/delivery/delivery_handler.h](src/modules/delivery/delivery_handler.h) | 定义 HTTP 请求入口、参数或响应。可从 `tm` 开始 |
| [src/modules/interview/interview_handler.h](src/modules/interview/interview_handler.h) | 定义 HTTP 请求入口、参数或响应 |
| [src/modules/jobs/jobs_handler.h](src/modules/jobs/jobs_handler.h) | 定义 HTTP 请求入口、参数或响应 |
| [src/modules/redstory/redstory_handler.h](src/modules/redstory/redstory_handler.h) | 定义 HTTP 请求入口、参数或响应 |
| [src/modules/resume/resume_handler.h](src/modules/resume/resume_handler.h) | 定义 HTTP 请求入口、参数或响应 |
| [src/modules/user/user_handler.h](src/modules/user/user_handler.h) | 定义 HTTP 请求入口、参数或响应 |
| [src/server/http_server.h](src/server/http_server.h) | HTTP 层。可从 `HttpServer` 开始 |

本页列出 49 个可阅读的项目文件，以及 0 个第三方或生成文件。
