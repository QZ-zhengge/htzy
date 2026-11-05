# 红途职引 Python 面试服务历史版 · 代码索引

从 [项目 README](README.md) 了解主流程。下表把当前快照中的源码、页面、脚本与主要配置按目录列出。职责根据代码声明与目录用途概括。

| 文件 | 职责或阅读线索 |
| --- | --- |
| [backend/app.py](backend/app.py) | 服务入口。可从 `allowed_file` 、 `login_required` 、 `decorated_function` 、 `log_ai_start` 开始 |
| [backend/config/config.py](backend/config/config.py) | 配置构建、应用启动或环境参数 |
| [backend/models/interview.py](backend/models/interview.py) | 定义接口或业务数据结构。可从 `Interview` 、 `__init__` 、 `save` 、 `start` 开始 |
| [backend/models/resume.py](backend/models/resume.py) | 定义接口或业务数据结构。可从 `Resume` 、 `__init__` 、 `save` 、 `get_by_id` 开始 |
| [backend/models/user.py](backend/models/user.py) | 定义接口或业务数据结构。可从 `User` 、 `__init__` 、 `save` 、 `update_last_login` 开始 |
| [backend/utils/database.py](backend/utils/database.py) | 处理存储连接、表结构或数据迁移。可从 `DatabaseConnection` 、 `__init__` 、 `connect` 、 `disconnect` 开始 |
| [backend/utils/image_analysis.py](backend/utils/image_analysis.py) | 图像分析。可从 `VolcengineClientImageMixin` 、 `__init__` 、 `_create_ws_url` 、 `_WsParam` 开始 |
| [backend/utils/text_analysis.py](backend/utils/text_analysis.py) | 文本分析。可从 `VolcengineClientTextMixin` 、 `__init__` 、 `_build_websocket_url` 、 `call_doubao_model` 开始 |
| [backend/utils/volcengine_client.py](backend/utils/volcengine_client.py) | 该目录下的业务实现或工具代码。可从 `VolcengineClient` 开始 |
| [backend/utils/volcengine_client_deepseek.py](backend/utils/volcengine_client_deepseek.py) | 该目录下的业务实现或工具代码。可从 `VolcengineClientTextMixin` 、 `__init__` 、 `call_doubao_model` 、 `analyze_resume` 开始 |
| [database/init_db.py](database/init_db.py) | 建库入口。可从 `create_database` 开始 |
| [frontend/templates/base.html](frontend/templates/base.html) | 实现页面、组件或用户交互 |
| [frontend/templates/dashboard.html](frontend/templates/dashboard.html) | 实现页面、组件或用户交互 |
| [frontend/templates/index.html](frontend/templates/index.html) | 实现页面、组件或用户交互 |
| [frontend/templates/interview.html](frontend/templates/interview.html) | 实现页面、组件或用户交互 |
| [frontend/templates/login.html](frontend/templates/login.html) | 实现页面、组件或用户交互 |
| [frontend/templates/register.html](frontend/templates/register.html) | 实现页面、组件或用户交互 |
| [frontend/templates/results.html](frontend/templates/results.html) | 实现页面、组件或用户交互 |
| [frontend/templates/resume.html](frontend/templates/resume.html) | 实现页面、组件或用户交互 |

本页列出 19 个可阅读的项目文件，以及 0 个第三方或生成文件。
