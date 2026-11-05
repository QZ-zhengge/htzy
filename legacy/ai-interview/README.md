# 红途职引 Python 面试服务历史版

保存早期独立面试与简历服务，便于解释 v2 前的系统边界。

## 代码从哪里读

| 入口 | 负责什么 |
| --- | --- |
| [backend/app.py](backend/app.py) | 服务入口 |
| [backend/models](backend/models) | 用户、简历与面试数据 |
| [backend/utils/text_analysis.py](backend/utils/text_analysis.py) | 文本分析 |
| [backend/utils/image_analysis.py](backend/utils/image_analysis.py) | 图像分析 |
| [database/init_db.py](database/init_db.py) | 建库入口 |

## 一条业务链路

入口接收简历或面试相关请求，模型层提供数据结构，分析工具负责把文本或图片交给对应服务。阅读配置和异常返回能看到旧版依赖边界。

## 运行与验证

从 .env.example 准备本地配置，按实际 Python 导入安装依赖，准备数据库与测试模型配置后启动。历史快照未重新联调。

## 面试可能会问

**为什么保留这一版？**

它记录了 AI 服务与 C++ 网站曾经分别开发的形态，也便于对照迁移中的接口和数据。

**模型输出格式不稳定怎么办？**

把解析和校验做成独立边界，失败时保留原始错误状态并给用户可理解的反馈。

## 继续阅读

- [逐文件代码索引](CODE_INDEX.md)
- [作品集展示与面试指南](https://github.com/QZ-zhengge/developer-portfolio/blob/main/docs/INTERVIEW.md)
