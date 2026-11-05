package com.example.v10.ui.chat

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Toast
import androidx.core.widget.addTextChangedListener
import androidx.fragment.app.Fragment
import androidx.recyclerview.widget.LinearLayoutManager
import com.example.v10.Config
import com.example.v10.api.providers.DeepSeekProvider
import com.example.v10.data.ChatMessage
import com.example.v10.data.ChatStorage
import com.example.v10.databinding.FragmentChatBinding

class ChatFragment : Fragment() {

    private var _binding: FragmentChatBinding? = null
    private val binding get() = _binding!!

    private lateinit var chatAdapter: ChatAdapter
    private val messages = mutableListOf<ChatMessage>()

    private val deepSeekProvider = DeepSeekProvider()
    private val systemPrompt = Config.SYSTEM_PROMPT

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        _binding = FragmentChatBinding.inflate(inflater, container, false)
        return binding.root
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        setupChatRecyclerView()
        setupInput()
        setupSendButton()
        setupNewTopicButton()

        loadHistory()
    }

    private fun setupNewTopicButton() {
        binding.newTopicButton.setOnClickListener {
            // 添加分割线
            val divider = ChatMessage(
                content = "—— 新话题 ——",
                isUser = false,
                isDivider = true
            )
            messages.add(divider)
            chatAdapter.submitList(messages.toList())
            binding.chatRecyclerView.post {
                binding.chatRecyclerView.scrollToPosition(messages.size - 1)
            }

            // 清空历史记录和记忆文件
            ChatStorage.clearHistory(requireContext())
            ChatStorage.generateMemoryFile(requireContext(), systemPrompt)

            Toast.makeText(context, "已开启新话题", Toast.LENGTH_SHORT).show()
        }
    }

    private fun loadHistory() {
        try {
            val history = ChatStorage.loadMessages(requireContext())
            messages.clear()
            messages.addAll(history)
            chatAdapter.submitList(messages.toList())

            if (messages.isNotEmpty()) {
                binding.chatRecyclerView.post {
                    binding.chatRecyclerView.scrollToPosition(messages.size - 1)
                }
            }

            ChatStorage.generateMemoryFile(requireContext(), systemPrompt)
        } catch (e: Exception) {
            e.printStackTrace()
        }
    }

    private fun setupChatRecyclerView() {
        chatAdapter = ChatAdapter()
        binding.chatRecyclerView.apply {
            layoutManager = LinearLayoutManager(context).apply {
                stackFromEnd = true
            }
            adapter = chatAdapter
        }
    }

    private fun setupInput() {
        binding.messageInput.addTextChangedListener {
            binding.sendButton.isEnabled = it?.isNotBlank() == true
        }
    }

    private fun setupSendButton() {
        binding.sendButton.setOnClickListener {
            val input = binding.messageInput.text?.toString()?.trim() ?: return@setOnClickListener
            if (input.isEmpty()) return@setOnClickListener

            val userMsg = ChatMessage(input, isUser = true)
            messages.add(userMsg)
            chatAdapter.submitList(messages.toList())
            binding.chatRecyclerView.post {
                binding.chatRecyclerView.scrollToPosition(messages.size - 1)
            }
            binding.messageInput.text?.clear()

            ChatStorage.appendMessage(requireContext(), userMsg)

            setLoading(true)
            val allMessages = buildList {
                // system prompt 必须是 role: "system"
                add(ChatMessage(systemPrompt, isUser = false, isSystem = true))
                addAll(messages.filter { !it.isDivider })
            }

            deepSeekProvider.sendMessage(allMessages) { result ->
                // 用 view?.post 代替 requireActivity().runOnUiThread
                // 避免 Fragment 离开 Activity 后调用 requireActivity() 崩溃
                view?.post {
                    if (!isAdded || _binding == null) return@post

                    setLoading(false)
                    result.onSuccess { reply ->
                        val aiMsg = ChatMessage(reply, isUser = false)
                        messages.add(aiMsg)
                        chatAdapter.submitList(messages.toList())
                        binding.chatRecyclerView.post {
                            binding.chatRecyclerView.scrollToPosition(messages.size - 1)
                        }

                        ChatStorage.appendMessage(requireContext(), aiMsg)
                        ChatStorage.generateMemoryFile(requireContext(), systemPrompt)
                    }.onFailure { error ->
                        Toast.makeText(context, "错误: ${error.message}", Toast.LENGTH_LONG).show()
                        if (messages.isNotEmpty()) {
                            messages.removeLast()
                            chatAdapter.submitList(messages.toList())
                        }
                    }
                }
            }
        }
    }

    private fun setLoading(loading: Boolean) {
        binding.sendButton.isEnabled = !loading
        binding.progressBar.visibility = if (loading) View.VISIBLE else View.GONE
        binding.messageInput.isEnabled = !loading
    }

    override fun onDestroyView() {
        super.onDestroyView()
        _binding = null
    }
}
