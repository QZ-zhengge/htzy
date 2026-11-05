<template>
  <div class="verify-page">
    <div class="verify-card">
      <div class="icon">📋</div>
      <h2>学生身份待验证</h2>
      <p>你的账号尚未通过学生身份验证，暂时无法使用简历管理和 AI 面试功能。</p>
      <p class="hint">请联系管理员将你加入学生花名册，或检查注册时填写的学号和姓名是否正确。</p>
      <div class="actions">
        <router-link to="/login" class="btn btn-outline" @click="logout">退出登录</router-link>
        <router-link to="/" class="btn btn-primary">返回首页</router-link>
      </div>
    </div>
  </div>
</template>

<script setup>
import { useUserStore } from '../stores/user'
import { useRouter } from 'vue-router'
const userStore = useUserStore()
const router = useRouter()
function logout() {
  userStore.logout()
  router.push('/login')
}
</script>

<style scoped>
.verify-page { min-height: 80vh; display: flex; align-items: center; justify-content: center; padding: 2rem; }
.verify-card { text-align: center; max-width: 480px; background: white; border-radius: 16px; padding: 3rem 2rem; box-shadow: 0 4px 20px rgba(0,0,0,0.08); }
.icon { font-size: 3rem; margin-bottom: 1rem; }
h2 { font-size: 1.5rem; font-weight: 700; color: #1f2937; margin-bottom: 0.75rem; }
p { color: #6b7280; line-height: 1.6; margin-bottom: 0.5rem; }
.hint { font-size: 0.875rem; color: #9ca3af; margin-top: 0.5rem; }
.actions { display: flex; gap: 1rem; justify-content: center; margin-top: 2rem; }
</style>
