<template>
  <div class="auth-page">
    <div class="auth-container">
      <div class="auth-card">
        <div class="auth-header">
          <h1 class="auth-title">注册账号</h1>
          <p class="auth-subtitle">创建账号，开始AI面试之旅</p>
        </div>
        
        <form class="auth-form" @submit.prevent="handleRegister">
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
            <label class="form-label">学号</label>
            <input type="text" class="form-input" v-model="form.student_id" placeholder="请输入学号" required />
          </div>

          <div class="form-group">
            <label class="form-label">真实姓名</label>
            <input type="text" class="form-input" v-model="form.real_name" placeholder="请输入与学籍一致的姓名" required />
          </div>

          <div class="form-group">
            <label class="form-label">班级</label>
            <input type="text" class="form-input" v-model="form.class_name" placeholder="例：计算机2401班" />
          </div>

          <div class="form-group">
            <label class="form-label">邮箱</label>
            <input type="email" class="form-input" v-model="form.email" placeholder="请输入邮箱" required />
          </div>

          <div class="form-group">
            <label class="form-label">邮箱验证码</label>
            <div class="code-row">
              <input type="text" class="form-input" v-model="form.verifyCode" placeholder="6位验证码" required />
              <button type="button" class="btn btn-outline btn-sm" @click="handleSendCode" :disabled="codeCooldown > 0 || !form.email">
                {{ codeCooldown > 0 ? codeCooldown + 's' : '发送验证码' }}
              </button>
            </div>
            <p v-if="codeMessage" class="code-hint" :class="{ error: codeError }">{{ codeMessage }}</p>
          </div>

          <div class="form-group">
            <label class="form-label">用户名</label>
            <input type="text" class="form-input" v-model="form.username" placeholder="3-20位字符" required />
          </div>

          <div class="form-group">
            <label class="form-label">密码</label>
            <input type="password" class="form-input" v-model="form.password" placeholder="至少6位" required />
          </div>

          <div class="form-group">
            <label class="form-label">确认密码</label>
            <input type="password" class="form-input" v-model="form.confirmPassword" placeholder="请再次输入密码" required />
          </div>
          
          <div v-if="error" class="error-message">{{ error }}</div>
          
          <button 
            type="submit" 
            class="btn btn-primary btn-block"
            :disabled="loading"
          >
            {{ loading ? '注册中...' : '注册' }}
          </button>
        </form>
        
        <div class="auth-footer">
          <p>已有账号？ <router-link to="/login">立即登录</router-link></p>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref } from 'vue'
import { useRouter } from 'vue-router'
import { authAPI } from '../api'
import { useUserStore } from '../stores/user'

const router = useRouter()
const userStore = useUserStore()

const form = ref({
  username: '',
  email: '',
  password: '',
  confirmPassword: '',
  student_id: '',
  real_name: '',
  class_name: '',
  verifyCode: '',
  email_verified: false
})

const loading = ref(false)
const error = ref('')
const codeCooldown = ref(0)
const codeMessage = ref('')
const codeError = ref(false)
let cooldownTimer = null

async function handleSendCode() {
  if (!form.value.email || !form.value.email.includes('@')) return
  codeMessage.value = '发送中...'
  codeError.value = false
  try {
    await authAPI.sendCode(form.value.email)
    codeMessage.value = '验证码已发送到您的邮箱（DEBUG模式可在控制台查看）'
    codeCooldown.value = 60
    cooldownTimer = setInterval(() => {
      if (codeCooldown.value > 0) codeCooldown.value--
      else clearInterval(cooldownTimer)
    }, 1000)
  } catch (e) {
    codeMessage.value = '发送失败: ' + e.message
    codeError.value = true
  }
}

async function handleRegister() {
  if (!form.value.username || !form.value.email || !form.value.password || !form.value.confirmPassword ||
      !form.value.student_id || !form.value.real_name) {
    error.value = '请填写完整信息（学号和姓名必填）'
    return
  }

  if (!form.value.verifyCode) {
    error.value = '请输入邮箱验证码'
    return
  }

  if (form.value.password !== form.value.confirmPassword) {
    error.value = '两次输入的密码不一致'
    return
  }

  if (form.value.password.length < 6) {
    error.value = '密码长度至少6位'
    return
  }

  loading.value = true
  error.value = ''

  try {
    // Verify code first
    const verifyResult = await authAPI.verifyCode(form.value.email, form.value.verifyCode)
    if (!verifyResult.success) {
      error.value = verifyResult.message || '验证码错误'
      loading.value = false
      return
    }
    form.value.email_verified = true

    const { confirmPassword, ...data } = form.value
    const result = await authAPI.register(data)
    userStore.setAuth(result.token, {
      id: result.user_id, username: data.username,
      role: 'student', is_verified: result.is_verified || false,
      student_id: data.student_id, real_name: data.real_name
    })
    router.push('/dashboard')
  } catch (err) {
    error.value = err.message || '注册失败，请重试'
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
}
.code-row { display: flex; gap: 0.5rem; }
.code-row .form-input { flex: 1; }
.btn-sm { padding: 0.375rem 0.75rem; font-size: 0.8125rem; white-space: nowrap; }
.code-hint { font-size: 0.8125rem; margin-top: 0.375rem; color: #059669; }
.code-hint.error { color: #ef4444; }

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