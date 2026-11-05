package com.example.v10.api.providers

import android.util.Log
import com.example.v10.Config
import okhttp3.*
import okhttp3.MediaType.Companion.toMediaType
import okhttp3.RequestBody.Companion.toRequestBody
import org.json.JSONArray
import org.json.JSONObject
import java.io.IOException
import java.util.concurrent.TimeUnit

private const val TAG = "DeepSeekProvider"

/**
 * AI Provider — 通过后端代理转发请求，保护 API Key 不暴露在客户端
 *
 * 用户手机 → AI Proxy Server → DeepSeek API
 *
 * ⚠️ serverUrl 在 Config.kt 中配置，指向你的后端地址
 */
class DeepSeekProvider {

    fun sendMessage(messages: List<com.example.v10.data.ChatMessage>, callback: (Result<String>) -> Unit) {
        val serverUrl = Config.SERVER_BASE_URL

        val client = OkHttpClient.Builder()
            .connectTimeout(60, TimeUnit.SECONDS)
            .readTimeout(120, TimeUnit.SECONDS)
            .writeTimeout(60, TimeUnit.SECONDS)
            .build()

        // 构建消息数组，正确处理 system / user / assistant 角色
        val messagesJson = JSONArray()
        messages.forEach { msg ->
            val role = when {
                msg.isSystem -> "system"
                msg.isUser -> "user"
                else -> "assistant"
            }
            messagesJson.put(JSONObject().put("role", role).put("content", msg.content))
        }

        // 请求体只含 messages，不含任何 Key
        val requestBody = JSONObject().apply {
            put("messages", messagesJson)
        }.toString()

        Log.d(TAG, "==> POST $serverUrl/chat")
        Log.d(TAG, "==> body: $requestBody")

        val request = Request.Builder()
            .url("$serverUrl/chat")
            .addHeader("Content-Type", "application/json")
            .post(requestBody.toRequestBody("application/json".toMediaType()))
            .build()

        client.newCall(request).enqueue(object : Callback {
            override fun onFailure(call: Call, e: IOException) {
                Log.e(TAG, "<== onFailure: ${e.message}")
                callback(Result.failure(e))
            }

            override fun onResponse(call: Call, response: Response) {
                val body: String
                try {
                    body = response.body?.string() ?: throw IOException("空响应体")
                    Log.d(TAG, "<== HTTP ${response.code} body: $body")
                } catch (e: Exception) {
                    // body 读取失败，body 已经为空，在这里直接回调失败
                    Log.e(TAG, "<== 读取响应体失败: ${e.message}")
                    callback(Result.failure(IOException("读取响应体失败: ${e.message}")))
                    return
                }

                // 解析 JSON
                val json: JSONObject
                try {
                    json = JSONObject(body)
                } catch (e: Exception) {
                    // body 不是 JSON 格式，说明后端返回了非 JSON 错误页或文本
                    Log.e(TAG, "<== JSON解析失败 body=$body")
                    callback(Result.failure(IOException("服务器返回非JSON格式: $body")))
                    return
                }

                // 优先检查 error 字段
                if (json.has("error")) {
                    val errMsg = json.getString("error")
                    Log.e(TAG, "<== server error: $errMsg")
                    callback(Result.failure(IOException("服务器错误: $errMsg")))
                    return
                }

                // 提取 choices[0].message.content
                val choices = json.optJSONArray("choices")
                if (choices == null || choices.length() == 0) {
                    Log.e(TAG, "<== choices为空或不存在. body=$body")
                    callback(Result.failure(IOException("响应缺少 choices: $body")))
                    return
                }

                val reply: String
                try {
                    reply = choices.getJSONObject(0)
                        .getJSONObject("message")
                        .getString("content")
                } catch (e: Exception) {
                    Log.e(TAG, "<== choices格式解析失败: ${e.message}")
                    callback(Result.failure(IOException("choices 格式解析失败: ${e.message}")))
                    return
                }

                Log.d(TAG, "<== reply: $reply")
                callback(Result.success(reply))
            }
        })
    }
}
