<template>
  <nav class="navbar">
    <div class="container flex items-center justify-between">
      <!-- Logo -->
      <router-link to="/" class="navbar-brand">
        <img src="/logo.png" alt="红途职引" class="navbar-logo" />
        <span class="navbar-title">红途职引</span>
      </router-link>

      <!-- 导航链接 -->
      <div class="navbar-nav">
        <router-link to="/" class="nav-link" :class="{ active: $route.path === '/' }">首页</router-link>
        <router-link to="/jobs" class="nav-link" :class="{ active: $route.path === '/jobs' }">岗位</router-link>
        <template v-if="isLoggedIn">
          <router-link to="/resume" class="nav-link" :class="{ active: $route.path === '/resume' }">简历</router-link>
          <router-link to="/dashboard" class="nav-link" :class="{ active: $route.path === '/dashboard' }">面试</router-link>
        </template>
        <router-link to="/red-guide" class="nav-link" :class="{ active: $route.path === '/red-guide' }">指引</router-link>
        <router-link v-if="isLoggedIn" to="/auto-delivery" class="nav-link" :class="{ active: $route.path === '/auto-delivery' }">自动投递</router-link>
      </div>

      <!-- 用户操作 -->
      <div class="navbar-actions">
        <template v-if="isLoggedIn">
          <div class="user-menu" @click="toggleDropdown">
            <div class="user-avatar">{{ username.charAt(0).toUpperCase() }}</div>
            <span class="user-name">{{ username }}</span>
            <svg class="dropdown-icon" viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
              <path d="M7 10l5 5 5-5H7z" fill="currentColor"/>
            </svg>
          </div>
          <div v-if="showDropdown" class="dropdown-menu">
            <router-link v-if="isAdmin" to="/admin" class="dropdown-item" @click="showDropdown = false">管理后台</router-link>
          <router-link to="/dashboard" class="dropdown-item" @click="showDropdown = false">个人中心</router-link>
            <div class="dropdown-divider"></div>
            <button class="dropdown-item" @click="handleLogout">退出登录</button>
          </div>
        </template>
        <template v-else>
          <router-link to="/login" class="navbar-login-btn">登录</router-link>
          <router-link to="/register" class="navbar-register-btn">注册</router-link>
        </template>
      </div>

      <!-- 移动端菜单按钮 -->
      <button class="mobile-menu-btn" @click="toggleMobileMenu">
        <svg viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
          <path d="M3 12h18M3 6h18M3 18h18" stroke="currentColor" stroke-width="2" stroke-linecap="round"/>
        </svg>
      </button>
    </div>

    <!-- 移动端菜单 -->
    <div v-if="showMobileMenu" class="mobile-menu">
      <router-link to="/" class="mobile-nav-link" @click="showMobileMenu = false">首页</router-link>
      <router-link to="/jobs" class="mobile-nav-link" @click="showMobileMenu = false">岗位</router-link>
      <template v-if="isLoggedIn">
        <router-link to="/resume" class="mobile-nav-link" @click="showMobileMenu = false">简历</router-link>
        <router-link to="/dashboard" class="mobile-nav-link" @click="showMobileMenu = false">面试</router-link>
      </template>
      <router-link to="/red-guide" class="mobile-nav-link" @click="showMobileMenu = false">指引</router-link>
      <div class="mobile-menu-divider"></div>
      <template v-if="isLoggedIn">
        <div class="mobile-user-info">
          <div class="user-avatar">{{ username.charAt(0).toUpperCase() }}</div>
          <span>{{ username }}</span>
        </div>
        <button class="mobile-nav-link" @click="handleLogout">退出登录</button>
      </template>
      <template v-else>
        <router-link to="/login" class="mobile-nav-link" @click="showMobileMenu = false">登录</router-link>
        <router-link to="/register" class="mobile-nav-link primary" @click="showMobileMenu = false">注册</router-link>
      </template>
    </div>
  </nav>
</template>

<script setup>
import { ref, computed } from 'vue'
import { useRouter } from 'vue-router'
import { useUserStore } from '../stores/user'

const router = useRouter()
const userStore = useUserStore()

const isLoggedIn = computed(() => userStore.isLoggedIn)
const isAdmin = computed(() => userStore.isAdmin)
const username = computed(() => userStore.username)

const showDropdown = ref(false)
const showMobileMenu = ref(false)

function toggleDropdown() {
  showDropdown.value = !showDropdown.value
}

function toggleMobileMenu() {
  showMobileMenu.value = !showMobileMenu.value
}

function handleLogout() {
  userStore.logout()
  showDropdown.value = false
  showMobileMenu.value = false
  router.push('/')
}

// 点击外部关闭下拉菜单
if (typeof document !== 'undefined') {
  document.addEventListener('click', (e) => {
    if (!e.target.closest('.user-menu') && !e.target.closest('.dropdown-menu')) {
      showDropdown.value = false
    }
  })
}
</script>

<style scoped>
.navbar {
  position: sticky;
  top: 0;
  z-index: 50;
  background-color: #0f172a;
  border-bottom: 1px solid #1e293b;
  box-shadow: 0 4px 12px rgba(0, 0, 0, 0.4);
}

.navbar .container {
  height: 64px;
  display: flex;
  align-items: center;
  justify-content: space-between;
}

.navbar-brand {
  display: flex;
  align-items: center;
  gap: 0.5rem;
  text-decoration: none;
}

.navbar-logo {
  width: 36px;
  height: 36px;
  border-radius: 50%;
  object-fit: cover;
  border: 1.5px solid rgba(255,255,255,0.2);
}

.navbar-title {
  font-size: 1.25rem;
  font-weight: 700;
  color: #22d3ee;
  letter-spacing: -0.02em;
}

.navbar-nav {
  display: flex;
  align-items: center;
  gap: 0.5rem;
}

.nav-link {
  padding: 0.5rem 1rem;
  font-size: 0.9375rem;
  font-weight: 500;
  color: #cbd5e1;
  text-decoration: none;
  border-radius: var(--radius-md);
  transition: var(--transition);
}

.nav-link:hover {
  color: #22d3ee;
  background-color: rgba(34, 211, 238, 0.08);
}

.nav-link.active {
  color: #22d3ee;
  background-color: rgba(34, 211, 238, 0.12);
}

.navbar-actions {
  display: flex;
  align-items: center;
  gap: 0.75rem;
  position: relative;
}

.user-menu {
  display: flex;
  align-items: center;
  gap: 0.5rem;
  padding: 0.5rem;
  cursor: pointer;
  border-radius: var(--radius-md);
  transition: var(--transition);
}

.user-menu:hover {
  background-color: rgba(255, 255, 255, 0.08);
}

.user-avatar {
  width: 32px;
  height: 32px;
  border-radius: 50%;
  background: linear-gradient(135deg, #0ea5e9, #06b6d4);
  color: white;
  display: flex;
  align-items: center;
  justify-content: center;
  font-weight: 600;
  font-size: 0.875rem;
}

.user-name {
  font-weight: 500;
  font-size: 0.9375rem;
  color: #e2e8f0;
}

.dropdown-icon {
  width: 20px;
  height: 20px;
  color: #94a3b8;
}

.dropdown-menu {
  position: absolute;
  top: 100%;
  right: 0;
  margin-top: 0.5rem;
  background-color: #1e293b;
  border-radius: var(--radius-lg);
  box-shadow: 0 10px 25px rgba(0, 0, 0, 0.4);
  min-width: 150px;
  z-index: 100;
  border: 1px solid #334155;
}

.dropdown-item {
  display: block;
  width: 100%;
  padding: 0.75rem 1rem;
  font-size: 0.9375rem;
  color: #cbd5e1;
  text-decoration: none;
  background: none;
  border: none;
  text-align: left;
  cursor: pointer;
  transition: var(--transition);
}

.dropdown-item:hover {
  background-color: rgba(34, 211, 238, 0.08);
  color: #22d3ee;
}

.dropdown-divider {
  height: 1px;
  background-color: #334155;
  margin: 0.5rem 0;
}

.mobile-menu-btn {
  display: none;
  background: none;
  border: none;
  padding: 0.5rem;
  cursor: pointer;
  color: #cbd5e1;
}

.mobile-menu-btn svg {
  width: 24px;
  height: 24px;
}

.mobile-menu {
  display: none;
  flex-direction: column;
  padding: 1rem;
  border-top: 1px solid #334155;
  background-color: #0f172a;
}

.mobile-nav-link {
  display: block;
  padding: 0.75rem 1rem;
  font-size: 1rem;
  font-weight: 500;
  color: #cbd5e1;
  text-decoration: none;
  border-radius: var(--radius-md);
  transition: var(--transition);
}

.mobile-nav-link:hover {
  background-color: rgba(34, 211, 238, 0.08);
  color: #22d3ee;
}

.mobile-nav-link.primary {
  background: linear-gradient(135deg, #0ea5e9, #06b6d4);
  color: white;
  text-align: center;
}

.mobile-menu-divider {
  height: 1px;
  background-color: #334155;
  margin: 0.5rem 0;
}

.mobile-user-info {
  display: flex;
  align-items: center;
  gap: 0.75rem;
  padding: 0.75rem 1rem;
  color: #e2e8f0;
}

@media (max-width: 768px) {
  .navbar-nav,
  .navbar-actions {
    display: none;
  }

  .mobile-menu-btn {
    display: block;
  }

  .mobile-menu {
    display: flex;
  }
}

.navbar-login-btn {
  padding: 0.5rem 1.25rem;
  border: 1.5px solid rgba(255,255,255,0.6);
  border-radius: 8px;
  color: white;
  font-size: 0.9375rem;
  font-weight: 500;
  text-decoration: none;
  transition: all 0.2s;
  background: transparent;
}
.navbar-login-btn:hover {
  background: rgba(255,255,255,0.15);
  border-color: white;
}
.navbar-register-btn {
  padding: 0.5rem 1.25rem;
  border: none;
  border-radius: 8px;
  color: white;
  font-size: 0.9375rem;
  font-weight: 600;
  text-decoration: none;
  transition: all 0.2s;
  background: #0ea5e9;
}
.navbar-register-btn:hover {
  background: #0284c7;
}
</style>