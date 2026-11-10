# 红途职引 AI 接入代理

早期客户端与模型服务之间的 Python 转发层。

## 项目结构

| 入口 | 负责什么 |
| --- | --- |
| [server.py](server.py) | 代理服务、路由及上游调用 |
| [requirements.txt](requirements.txt) | Python 依赖 |

## 运行与验证

在隔离环境安装 requirements.txt，按 server.py 配置自有测试上游，再启动服务。归档未验证第三方连通性。
