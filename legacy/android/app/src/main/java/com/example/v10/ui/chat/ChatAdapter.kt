package com.example.v10.ui.chat

import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import androidx.recyclerview.widget.DiffUtil
import androidx.recyclerview.widget.ListAdapter
import androidx.recyclerview.widget.RecyclerView
import com.example.v10.R
import com.example.v10.data.ChatMessage

class ChatAdapter : ListAdapter<ChatMessage, RecyclerView.ViewHolder>(MessageDiffCallback()) {

    companion object {
        private const val VIEW_TYPE_USER = 0
        private const val VIEW_TYPE_AI = 1
        private const val VIEW_TYPE_DIVIDER = 2
    }

    override fun getItemViewType(position: Int): Int {
        return when {
            getItem(position).isDivider -> VIEW_TYPE_DIVIDER
            getItem(position).isUser -> VIEW_TYPE_USER
            else -> VIEW_TYPE_AI
        }
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): RecyclerView.ViewHolder {
        return when (viewType) {
            VIEW_TYPE_DIVIDER -> {
                val view = LayoutInflater.from(parent.context)
                    .inflate(R.layout.item_chat_divider, parent, false)
                DividerViewHolder(view)
            }
            VIEW_TYPE_USER -> {
                val view = LayoutInflater.from(parent.context)
                    .inflate(R.layout.item_chat_user, parent, false)
                MessageViewHolder(view)
            }
            else -> {
                val view = LayoutInflater.from(parent.context)
                    .inflate(R.layout.item_chat_ai, parent, false)
                MessageViewHolder(view)
            }
        }
    }

    override fun onBindViewHolder(holder: RecyclerView.ViewHolder, position: Int) {
        val message = getItem(position)
        if (holder is MessageViewHolder && !message.isDivider) {
            holder.bind(message)
        }
    }

    class MessageViewHolder(itemView: View) : RecyclerView.ViewHolder(itemView) {
        private val messageText: TextView = itemView.findViewById(R.id.messageText)
        fun bind(message: ChatMessage) {
            messageText.text = message.content
        }
    }

    class DividerViewHolder(itemView: View) : RecyclerView.ViewHolder(itemView)

    class MessageDiffCallback : DiffUtil.ItemCallback<ChatMessage>() {
        override fun areItemsTheSame(oldItem: ChatMessage, newItem: ChatMessage): Boolean {
            // 按「内容 + 是否用户 + 是否分割线」判断是否为同一逻辑条目
            // 不再用 timestamp（同一毫秒发两条会冲突）
            return oldItem.content == newItem.content
                    && oldItem.isUser == newItem.isUser
                    && oldItem.isDivider == newItem.isDivider
                    && oldItem.isSystem == newItem.isSystem
        }

        override fun areContentsTheSame(oldItem: ChatMessage, newItem: ChatMessage): Boolean {
            return oldItem == newItem
        }
    }
}
