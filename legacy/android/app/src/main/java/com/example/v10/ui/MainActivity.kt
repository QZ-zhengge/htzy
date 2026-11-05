package com.example.v10.ui

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.fragment.app.Fragment
import com.example.v10.R
import com.example.v10.databinding.ActivityMainBinding
import com.example.v10.ui.browser.BrowserFragment
import com.example.v10.ui.chat.ChatFragment


class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding
    private var browserFragment: BrowserFragment? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        setupBottomNavigation()

        // Load default fragment
        if (savedInstanceState == null) {
            loadFragment(ChatFragment())
        }
    }

    private fun setupBottomNavigation() {
        binding.bottomNavigation.setOnItemSelectedListener { item ->
            when (item.itemId) {
                R.id.nav_chat -> {
                    loadFragment(ChatFragment())
                    true
                }
                R.id.nav_browser -> {
                    if (browserFragment == null) {
                        browserFragment = BrowserFragment()
                    }
                    loadFragment(browserFragment!!)
                    true
                }
                else -> false
            }
        }
    }

    private fun loadFragment(fragment: Fragment) {
        supportFragmentManager.beginTransaction()
            .replace(R.id.fragmentContainer, fragment)
            .commit()
    }

    @Deprecated("Deprecated in Java")
    override fun onBackPressed() {
        val currentFragment = supportFragmentManager.findFragmentById(R.id.fragmentContainer)
        if (currentFragment is BrowserFragment) {
            if (currentFragment.onBackPressed()) {
                return
            }
        }
        super.onBackPressed()
    }
}
