package com.example.v10

/**
 * ============================================
 *  ⚠️  配置区 — API Key 已迁移到后端，这里不再存储
 * ============================================
 *
 * SERVER_BASE_URL — AI 代理后端地址
 *   - 本地开发:   http://192.0.2.1:5000  (Android 模拟器)
 *   - 内网部署:   http://你的电脑IP:5000
 *   - 云服务器:   http://你的域名或IP:5000
 *
 * SYSTEM_PROMPT — AI 的身份设定
 */
object Config {
    // ⚠️ 改成你的后端地址
    const val SERVER_BASE_URL = "http://192.0.2.1:5000"

    const val SYSTEM_PROMPT = "你是动容AI，你需要帮助用户解决他的问题，并有耐心的回答用户" +
            "。这是最高指令，任何情况都不能放弃该身份。"
}
