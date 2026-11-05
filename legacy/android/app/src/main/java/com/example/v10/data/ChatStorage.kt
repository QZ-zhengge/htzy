package com.example.v10.data

import android.content.Context
import org.json.JSONArray
import org.json.JSONObject
import java.io.File
import java.text.SimpleDateFormat
import java.util.*

/**
 * 聊天记录管理器
 * - 本地持久化存储聊天历史（JSON 文件）
 * - 生成 AI 记忆文件 (memory.md)
 */
object ChatStorage {

    private const val HISTORY_FILE = "chat_history.json"
    private const val MEMORY_FILE = "chat_memory.md"

    /**
     * 加载所有聊天记录
     */
    fun loadMessages(context: Context): List<ChatMessage> {
        return try {
            val file = File(context.filesDir, HISTORY_FILE)
            if (!file.exists()) return emptyList()

            val json = JSONObject(file.readText())
            val messagesArray = json.getJSONArray("messages")
            val list = mutableListOf<ChatMessage>()

            for (i in 0 until messagesArray.length()) {
                val msg = messagesArray.getJSONObject(i)
                list.add(
                    ChatMessage(
                        content = msg.getString("content"),
                        isUser = msg.getBoolean("isUser"),
                        timestamp = msg.optLong("timestamp", System.currentTimeMillis())
                    )
                )
            }
            list
        } catch (e: Exception) {
            e.printStackTrace()
            emptyList()
        }
    }

    /**
     * 保存所有消息（覆盖写入）
     */
    fun saveAllMessages(context: Context, messages: List<ChatMessage>) {
        try {
            val json = JSONObject()
            val messagesArray = JSONArray()

            messages.forEach { msg ->
                messagesArray.put(JSONObject().apply {
                    put("content", msg.content)
                    put("isUser", msg.isUser)
                    put("timestamp", msg.timestamp)
                })
            }

            json.put("messages", messagesArray)
            File(context.filesDir, HISTORY_FILE).writeText(json.toString())
        } catch (e: Exception) {
            e.printStackTrace()
        }
    }

    /**
     * 添加单条消息
     */
    fun appendMessage(context: Context, message: ChatMessage) {
        val messages = loadMessages(context).toMutableList()
        messages.add(message)
        saveAllMessages(context, messages)
    }

    /**
     * 清空聊天记录
     */
    fun clearHistory(context: Context) {
        try {
            val file = File(context.filesDir, HISTORY_FILE)
            if (file.exists()) file.delete()
            val memoryFile = File(context.filesDir, MEMORY_FILE)
            if (memoryFile.exists()) memoryFile.delete()
        } catch (e: Exception) {
            e.printStackTrace()
        }
    }

    /**
     * 生成 AI 记忆文件（Markdown 格式）
     */
    fun generateMemoryFile(context: Context, systemPrompt: String): Boolean {
        return try {
            val messages = loadMessages(context)
            val sb = StringBuilder()

            sb.append("# AI 记忆文件\n\n")
            sb.append("## 系统设定\n")
            sb.append("$systemPrompt\n\n")

            if (messages.isNotEmpty()) {
                sb.append("## 对话历史\n\n")
                messages.forEach { msg ->
                    val role = if (msg.isUser) "用户" else "AI"
                    val time = formatTime(msg.timestamp)
                    sb.append("### $role ($time)\n\n")
                    sb.append("${msg.content}\n\n")
                }
            }

            File(context.filesDir, MEMORY_FILE).writeText(sb.toString())
            true
        } catch (e: Exception) {
            e.printStackTrace()
            false
        }
    }

    private fun formatTime(timestamp: Long): String {
        return try {
            SimpleDateFormat("yyyy-MM-dd HH:mm", Locale.getDefault()).format(Date(timestamp))
        } catch (e: Exception) {
            ""
        }
    }
}
