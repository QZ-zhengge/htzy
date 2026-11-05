<template>
  <div class="auth-page">
    <div class="auth-container">
      <div class="auth-card">
        <div class="auth-header">
          <h1 class="auth-title">欢迎登录</h1>
          <p class="auth-subtitle">登录AI面试系统，继续你的面试准备</p>
        </div>
        
        <form class="auth-form" @submit.prevent="handleLogin">
          <div class="form-group">
            <label class="form-label">用户名</label>
            <input 
              type="text" 
              class="form-input" 
              v-model="form.username"
              placeholder="请输入用户名"
              required
            />
          </div>
          
          <div class="form-group">
            <label class="form-label">密码</label>
            <input 
              type="password" 
              class="form-input" 
              v-model="form.password"
              placeholder="请输入密码"
              required
            />
          </div>
          
          <div v-if="error" class="error-message">{{ error }}</div>
          
          <button 
            type="submit" 
            class="btn btn-primary btn-block"
            :disabled="loading"
          >
            {{ loading ? '登录中...' : '登录' }}
          </button>
        </form>
        
        <div class="auth-footer">
          <p>还没有账号？ <router-link to="/register">立即注册</router-link></p>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref } from 'vue'
import { useRouter, useRoute } from 'vue-router'
import { authAPI } from '../api'
import { useUserStore } from '../stores/user'

const router = useRouter()
const route = useRoute()
const userStore = useUserStore()

const form = ref({
  username: '',
  password: ''
})

const loading = ref(false)
const error = ref('')

async function handleLogin() {
  if (!form.value.username || !form.value.password) {
    error.value = '请填写完整信息'
    return
  }
  
  loading.value = true
  error.value = ''
  
  try {
    const data = await authAPI.login(form.value)
    userStore.setAuth(data.token, { id: data.user_id, username: data.username, role: data.role || 'student', is_verified: data.is_verified || false })
    
    // 跳转到之前想访问的页面或仪表板
    const redirect = route.query.redirect || '/dashboard'
    router.push(redirect)
  } catch (err) {
    error.value = err.message || '登录失败，请重试'
  } finally {
    loading.value = false
  }
}
</script>

<style scoped>
.auth-page {
  min-height: calc(100vh - 64px);
  display: flex;
  align-items: center;
  justify-content: center;
  background: var(--background);
  padding: 2rem 1rem;
}

.auth-container {
  width: 100%;
  max-width: 400px;
}

.auth-card {
  background: white;
  border-radius: 1rem;
  box-shadow: var(--shadow-xl);
  overflow: hidden;
}

.auth-header {
  background: linear-gradient(135deg, #0f172a, #1e293b);
  padding: 2rem;
  text-align: center;
  color: white;
  border-bottom: 2px solid #22d3ee;
}

.auth-title {
  font-size: 1.75rem;
  font-weight: 700;
  margin-bottom: 0.5rem;
}

.auth-subtitle {
  font-size: 0.9375rem;
  opacity: 0.9;
}

.auth-form {
  padding: 2rem;
}

.error-message {
  background-color: rgba(239, 68, 68, 0.1);
  color: var(--danger);
  padding: 0.75rem 1rem;
  border-radius: var(--radius-md);
  margin-bottom: 1rem;
  font-size: 0.875rem;
}

.btn-block {
  width: 100%;
  margin-top: 1rem;
}

.auth-footer {
  padding: 1.5rem 2rem;
  background-color: var(--background);
  text-align: center;
  border-top: 1px solid var(--border-color);
}

.auth-footer p {
  font-size: 0.9375rem;
  color: var(--text-secondary);
}

.auth-footer a {
  color: var(--primary);
  font-weight: 500;
}

.auth-footer a:hover {
  text-decoration: underline;
}
</style>