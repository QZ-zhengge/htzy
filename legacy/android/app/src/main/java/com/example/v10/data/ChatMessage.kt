package com.example.v10.data

data class ChatMessage(
    val content: String,
    val isUser: Boolean,
    val timestamp: Long = System.currentTimeMillis(),
    val isDivider: Boolean = false,
    val isSystem: Boolean = false
)
