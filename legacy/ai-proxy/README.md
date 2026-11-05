# 红途职引 AI 接入代理

早期客户端与模型服务之间的 Python 转发层。

## 代码从哪里读

| 入口 | 负责什么 |
| --- | --- |
| [server.py](server.py) | 代理服务、路由及上游调用 |
| [requirements.txt](requirements.txt) | Python 依赖 |

## 一条业务链路

从 server.py 的路由查看输入整理、上游请求和响应返回。该目录适合展示外部接口如何集中管理。

## 运行与验证

在隔离环境安装 requirements.txt，按 server.py 配置自有测试上游，再启动服务。归档未验证第三方连通性。

## 面试可能会问

**为什么把模型调用放服务端？**

集中保管凭据和记录错误，客户端只依赖自己的 API。

**代理要观察什么指标？**

上游延迟、失败率、超时和重试次数可定位问题来自代理还是模型服务。

## 继续阅读

- [逐文件代码索引](CODE_INDEX.md)
- [作品集展示与面试指南](https://github.com/QZ-zhengge/developer-portfolio/blob/main/docs/INTERVIEW.md)
