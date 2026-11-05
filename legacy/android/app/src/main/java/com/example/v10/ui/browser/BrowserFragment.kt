package com.example.v10.ui.browser

import android.content.Context
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import com.example.v10.databinding.FragmentBrowserBinding

class BrowserFragment : Fragment() {

    private var _binding: FragmentBrowserBinding? = null
    private val binding get() = _binding!!

    companion object {
        const val DEFAULT_URL = "https://www.example.com"
    }

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        _binding = FragmentBrowserBinding.inflate(inflater, container, false)
        return binding.root
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        val prefs = requireContext().getSharedPreferences("ai_assistant", Context.MODE_PRIVATE)
        val savedUrl = prefs.getString("fixed_url", DEFAULT_URL) ?: DEFAULT_URL

        // WebView settings
        binding.webView.apply {
            settings.apply {
                javaScriptEnabled = true
                domStorageEnabled = true
                loadWithOverviewMode = true
                useWideViewPort = true
            }

            webViewClient = android.webkit.WebViewClient()

            webChromeClient = object : android.webkit.WebChromeClient() {
                override fun onProgressChanged(view: android.webkit.WebView?, newProgress: Int) {
                    binding.progressBar.progress = newProgress
                    binding.progressBar.visibility = if (newProgress < 100) View.VISIBLE else View.GONE
                }
            }

            loadUrl(savedUrl)
        }

        // Refresh button
        binding.refreshButton.setOnClickListener {
            binding.webView.reload()
        }

        // Open URL settings
        binding.settingsButton.setOnClickListener {
            showUrlSettingsDialog(savedUrl)
        }
    }

    private fun showUrlSettingsDialog(currentUrl: String) {
        val dialogView = layoutInflater.inflate(
            com.example.v10.R.layout.dialog_url_settings,
            null
        )
        val editText = dialogView.findViewById<com.google.android.material.textfield.TextInputEditText>(
            com.example.v10.R.id.urlInput
        )
        editText.setText(currentUrl)

        androidx.appcompat.app.AlertDialog.Builder(requireContext())
            .setTitle("设置固定网页")
            .setMessage("设置浏览器启动时加载的固定网址")
            .setView(dialogView)
            .setPositiveButton("保存并跳转") { _, _ ->
                val newUrl = editText.text?.toString()?.trim() ?: ""
                if (newUrl.isNotEmpty()) {
                    val finalUrl = if (newUrl.startsWith("http://") || newUrl.startsWith("https://")) {
                        newUrl
                    } else {
                        "https://$newUrl"
                    }
                    requireContext().getSharedPreferences("ai_assistant", Context.MODE_PRIVATE)
                        .edit()
                        .putString("fixed_url", finalUrl)
                        .apply()
                    binding.webView.loadUrl(finalUrl)
                }
            }
            .setNegativeButton("取消", null)
            .show()
    }

    fun onBackPressed(): Boolean {
        return if (binding.webView.canGoBack()) {
            binding.webView.goBack()
            true
        } else {
            false
        }
    }

    override fun onDestroyView() {
        super.onDestroyView()
        _binding = null
    }
}
