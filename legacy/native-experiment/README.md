# 红途职引 C++ 客户端试验

保存桌面端的 CMake 骨架和构建预设。

## 代码从哪里读

| 入口 | 负责什么 |
| --- | --- |
| [HTZY_App.cpp](HTZY_App.cpp) | 原型入口 |
| [HTZY_App.h](HTZY_App.h) | 声明 |
| [CMakeLists.txt](CMakeLists.txt) | 目标与编译规则 |
| [CMakePresets.json](CMakePresets.json) | 构建预设 |

## 一条业务链路

从 CMake 目标追踪到原型入口，展示新工程起步时如何约定工具链和编译配置。

## 运行与验证

准备 CMake 和本机 C++ 编译器，核对预设中的本机路径后配置构建。此目录为早期试验。

## 面试可能会问

**这个目录演示什么？**

演示桌面端工程如何启动和组织构建，业务展示应打开 v2 或 Android 目录。

**为什么保留预设？**

预设使开发者共享生成器和构建选项，迁移时仍需处理机器专属路径。

## 继续阅读

- [逐文件代码索引](CODE_INDEX.md)
- [作品集展示与面试指南](https://github.com/QZ-zhengge/developer-portfolio/blob/main/docs/INTERVIEW.md)
