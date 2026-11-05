# 红途职引 Android 历史版

展示早期移动端的聊天、网页浏览与本地会话。

## 代码从哪里读

| 入口 | 负责什么 |
| --- | --- |
| [app/src/main/java/com/example/v10/ui/MainActivity.kt](app/src/main/java/com/example/v10/ui/MainActivity.kt) | Activity 容器 |
| [app/src/main/java/com/example/v10/ui/chat/ChatFragment.kt](app/src/main/java/com/example/v10/ui/chat/ChatFragment.kt) | 聊天界面 |
| [app/src/main/java/com/example/v10/api/providers/AiProviders.kt](app/src/main/java/com/example/v10/api/providers/AiProviders.kt) | 模型服务适配 |
| [app/src/main/java/com/example/v10/data/ChatStorage.kt](app/src/main/java/com/example/v10/data/ChatStorage.kt) | 聊天存储 |

## 一条业务链路

Activity 承载各页面。聊天 Fragment 通过 provider 发起请求并写入聊天存储，浏览 Fragment 负责网页交互。

## 运行与验证

用 Android Studio 导入，准备 Android SDK、Gradle 和自有 API 配置。历史版本未重新打包。

## 面试可能会问

**为何将 provider 和页面分开？**

不同模型服务的数据结构和请求形式可以封装在适配层，页面集中处理输入、加载和错误状态。

**Activity 重建要验证什么？**

验证旋转屏幕后会话恢复、请求取消和列表滚动是否符合预期。

## 继续阅读

- [逐文件代码索引](CODE_INDEX.md)
- [作品集展示与面试指南](https://github.com/QZ-zhengge/developer-portfolio/blob/main/docs/INTERVIEW.md)
