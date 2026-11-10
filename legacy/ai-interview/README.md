# 红途职引 Python 面试服务历史版

保存早期独立面试与简历服务，便于解释 v2 前的系统边界。

## 项目结构

| 入口 | 负责什么 |
| --- | --- |
| [backend/app.py](backend/app.py) | 服务入口 |
| [backend/models](backend/models) | 用户、简历与面试数据 |
| [backend/utils/text_analysis.py](backend/utils/text_analysis.py) | 文本分析 |
| [backend/utils/image_analysis.py](backend/utils/image_analysis.py) | 图像分析 |
| [database/init_db.py](database/init_db.py) | 建库入口 |

## 运行与验证

从 .env.example 准备本地配置，按实际 Python 导入安装依赖，准备数据库与测试模型配置后启动。历史快照未重新联调。
