# 红途职引 Android 历史版 · 代码索引

从 [项目 README](README.md) 了解主流程。下表把当前快照中的源码、页面、脚本与主要配置按目录列出。职责根据代码声明与目录用途概括。

| 文件 | 职责或阅读线索 |
| --- | --- |
| [app/build.gradle.kts](app/build.gradle.kts) | 配置构建、应用启动或环境参数 |
| [app/src/androidTest/java/com/example/v10/ExampleInstrumentedTest.java](app/src/androidTest/java/com/example/v10/ExampleInstrumentedTest.java) | 验证功能或回归场景。可从 `ExampleInstrumentedTest` 开始 |
| [app/src/main/AndroidManifest.xml](app/src/main/AndroidManifest.xml) | 提供工程配置、界面资源或结构化数据 |
| [app/src/main/java/com/example/v10/Config.kt](app/src/main/java/com/example/v10/Config.kt) | 配置构建、应用启动或环境参数 |
| [app/src/main/java/com/example/v10/api/providers/AiProviders.kt](app/src/main/java/com/example/v10/api/providers/AiProviders.kt) | 模型服务适配。可从 `DeepSeekProvider` 、 `sendMessage` 、 `onFailure` 、 `onResponse` 开始 |
| [app/src/main/java/com/example/v10/data/ChatMessage.kt](app/src/main/java/com/example/v10/data/ChatMessage.kt) | 该目录下的业务实现或工具代码。可从 `ChatMessage` 开始 |
| [app/src/main/java/com/example/v10/data/ChatStorage.kt](app/src/main/java/com/example/v10/data/ChatStorage.kt) | 聊天存储。可从 `loadMessages` 、 `saveAllMessages` 、 `appendMessage` 、 `clearHistory` 开始 |
| [app/src/main/java/com/example/v10/ui/MainActivity.kt](app/src/main/java/com/example/v10/ui/MainActivity.kt) | Activity 容器。可从 `MainActivity` 、 `onCreate` 、 `setupBottomNavigation` 、 `loadFragment` 开始 |
| [app/src/main/java/com/example/v10/ui/browser/BrowserFragment.kt](app/src/main/java/com/example/v10/ui/browser/BrowserFragment.kt) | 该目录下的业务实现或工具代码。可从 `BrowserFragment` 、 `onCreateView` 、 `onViewCreated` 、 `onProgressChanged` 开始 |
| [app/src/main/java/com/example/v10/ui/chat/ChatAdapter.kt](app/src/main/java/com/example/v10/ui/chat/ChatAdapter.kt) | 该目录下的业务实现或工具代码。可从 `ChatAdapter` 、 `getItemViewType` 、 `onCreateViewHolder` 、 `onBindViewHolder` 开始 |
| [app/src/main/java/com/example/v10/ui/chat/ChatFragment.kt](app/src/main/java/com/example/v10/ui/chat/ChatFragment.kt) | 聊天界面。可从 `ChatFragment` 、 `onCreateView` 、 `onViewCreated` 、 `setupNewTopicButton` 开始 |
| [app/src/main/res/drawable/ic_launcher_background.xml](app/src/main/res/drawable/ic_launcher_background.xml) | 提供工程配置、界面资源或结构化数据 |
| [app/src/main/res/drawable/ic_launcher_foreground.xml](app/src/main/res/drawable/ic_launcher_foreground.xml) | 提供工程配置、界面资源或结构化数据 |
| [app/src/main/res/layout/activity_main.xml](app/src/main/res/layout/activity_main.xml) | 提供工程配置、界面资源或结构化数据 |
| [app/src/main/res/layout/dialog_ai_settings.xml](app/src/main/res/layout/dialog_ai_settings.xml) | 提供工程配置、界面资源或结构化数据 |
| [app/src/main/res/layout/dialog_api_key.xml](app/src/main/res/layout/dialog_api_key.xml) | 提供工程配置、界面资源或结构化数据 |
| [app/src/main/res/layout/dialog_url_settings.xml](app/src/main/res/layout/dialog_url_settings.xml) | 提供工程配置、界面资源或结构化数据 |
| [app/src/main/res/layout/fragment_browser.xml](app/src/main/res/layout/fragment_browser.xml) | 提供工程配置、界面资源或结构化数据 |
| [app/src/main/res/layout/fragment_chat.xml](app/src/main/res/layout/fragment_chat.xml) | 提供工程配置、界面资源或结构化数据 |
| [app/src/main/res/layout/item_chat_ai.xml](app/src/main/res/layout/item_chat_ai.xml) | 提供工程配置、界面资源或结构化数据 |
| [app/src/main/res/layout/item_chat_divider.xml](app/src/main/res/layout/item_chat_divider.xml) | 提供工程配置、界面资源或结构化数据 |
| [app/src/main/res/layout/item_chat_user.xml](app/src/main/res/layout/item_chat_user.xml) | 提供工程配置、界面资源或结构化数据 |
| [app/src/main/res/layout/item_dropdown.xml](app/src/main/res/layout/item_dropdown.xml) | 提供工程配置、界面资源或结构化数据 |
| [app/src/main/res/menu/bottom_nav_menu.xml](app/src/main/res/menu/bottom_nav_menu.xml) | 提供工程配置、界面资源或结构化数据 |
| [app/src/main/res/mipmap-anydpi-v26/ic_launcher.xml](app/src/main/res/mipmap-anydpi-v26/ic_launcher.xml) | 提供工程配置、界面资源或结构化数据 |
| [app/src/main/res/mipmap-anydpi-v26/ic_launcher_round.xml](app/src/main/res/mipmap-anydpi-v26/ic_launcher_round.xml) | 提供工程配置、界面资源或结构化数据 |
| [app/src/main/res/values-night/themes.xml](app/src/main/res/values-night/themes.xml) | 提供工程配置、界面资源或结构化数据 |
| [app/src/main/res/values/colors.xml](app/src/main/res/values/colors.xml) | 提供工程配置、界面资源或结构化数据 |
| [app/src/main/res/values/strings.xml](app/src/main/res/values/strings.xml) | 提供工程配置、界面资源或结构化数据 |
| [app/src/main/res/values/themes.xml](app/src/main/res/values/themes.xml) | 提供工程配置、界面资源或结构化数据 |
| [app/src/main/res/xml/backup_rules.xml](app/src/main/res/xml/backup_rules.xml) | 提供工程配置、界面资源或结构化数据 |
| [app/src/main/res/xml/data_extraction_rules.xml](app/src/main/res/xml/data_extraction_rules.xml) | 提供工程配置、界面资源或结构化数据 |
| [app/src/test/java/com/example/v10/ExampleUnitTest.java](app/src/test/java/com/example/v10/ExampleUnitTest.java) | 验证功能或回归场景。可从 `ExampleUnitTest` 开始 |
| [build.gradle.kts](build.gradle.kts) | 配置构建、应用启动或环境参数 |
| [gradlew.bat](gradlew.bat) | 提供运行、构建或部署辅助 |
| [settings.gradle.kts](settings.gradle.kts) | 配置构建、应用启动或环境参数 |

本页列出 36 个可阅读的项目文件，以及 0 个第三方或生成文件。
