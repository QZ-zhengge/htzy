<template>
  <div class="admin-login-page">
    <div class="login-container">
      <div class="login-header">
        <h1>管理后台</h1>
        <p>红途职引 · 管理系统</p>
      </div>
      <form @submit.prevent="handleLogin" class="login-form">
        <div class="form-group">
          <label>用户名</label>
          <input type="text" v-model="username" placeholder="管理员用户名" required />
        </div>
        <div class="form-group">
          <label>密码</label>
          <input type="password" v-model="password" placeholder="密码" required />
        </div>
        <p v-if="error" class="error-text">{{ error }}</p>
        <button type="submit" class="btn btn-primary btn-block" :disabled="loading">
          {{ loading ? '登录中...' : '登录' }}
        </button>
      </form>
      <div class="login-footer">
        <router-link to="/">返回首页</router-link>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref } from 'vue'
import { useRouter } from 'vue-router'
import { useUserStore } from '../../stores/user'
import { authAPI } from '../../api'

const router = useRouter()
const userStore = useUserStore()
const username = ref('')
const password = ref('')
const error = ref('')
const loading = ref(false)

async function handleLogin() {
  error.value = ''
  loading.value = true
  try {
    const result = await authAPI.login({ username: username.value, password: password.value })
    if (result.role !== 'admin' && result.role !== 'super_admin') {
      error.value = '此账号不是管理员账号'
      loading.value = false
      return
    }
    userStore.setAuth(result.token, { id: result.user_id, username: result.username, role: result.role, is_verified: true })
    router.push('/admin')
  } catch (err) {
    error.value = err.message || '登录失败'
  } finally {
    loading.value = false
  }
}
</script>

<style scoped>
.admin-login-page { min-height: 100vh; display: flex; align-items: center; justify-content: center; background: linear-gradient(135deg, #0f172a, #1e293b); }
.login-container { background: white; border-radius: 16px; padding: 2.5rem; width: 400px; max-width: 90vw; box-shadow: 0 20px 60px rgba(0,0,0,0.3); }
.login-header { text-align: center; margin-bottom: 2rem; }
.login-header h1 { font-size: 1.5rem; font-weight: 700; color: #1f2937; }
.login-header p { color: #6b7280; margin-top: 0.25rem; }
.form-group { margin-bottom: 1rem; }
.form-group label { display: block; font-size: 0.875rem; font-weight: 500; color: #374151; margin-bottom: 0.375rem; }
.form-group input { width: 100%; padding: 0.625rem 0.75rem; border: 1px solid #d1d5db; border-radius: 8px; font-size: 0.9375rem; }
.form-group input:focus { outline: none; border-color: #0ea5e9; box-shadow: 0 0 0 3px rgba(14,165,233,0.1); }
.btn-block { width: 100%; }
.error-text { color: #ef4444; font-size: 0.875rem; margin-bottom: 0.75rem; }
.login-footer { text-align: center; margin-top: 1.5rem; }
.login-footer a { color: #6b7280; font-size: 0.875rem; }
</style>
