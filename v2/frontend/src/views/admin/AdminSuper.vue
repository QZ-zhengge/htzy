<template>
  <div class="admin-page">
    <div class="admin-sidebar">
      <div class="sidebar-logo">红途职引 · 管理后台</div>
      <nav class="sidebar-nav">
        <router-link to="/admin" class="nav-item" active-class="active">概览</router-link>
        <router-link to="/admin/students" class="nav-item" active-class="active">学生管理</router-link>
        <router-link to="/admin/scores" class="nav-item" active-class="active">成绩管理</router-link>
        <router-link to="/admin/super" class="nav-item" active-class="active">管理员管理</router-link>
      </nav>
      <div class="sidebar-footer">
        <router-link to="/" class="nav-item">返回前台</router-link>
        <button class="nav-item logout-btn" @click="handleLogout">退出登录</button>
      </div>
    </div>
    <div class="admin-content">
      <div class="admin-header">
        <h2>管理员管理（超级管理员）</h2>
      </div>

      <div class="card">
        <h3>添加新管理员</h3>
        <form @submit.prevent="handleAdd" class="add-form">
          <div class="form-row">
            <div class="form-group">
              <label>用户名</label>
              <input v-model="form.username" required placeholder="3-20位字符" />
            </div>
            <div class="form-group">
              <label>密码</label>
              <input type="password" v-model="form.password" required placeholder="至少6位" />
            </div>
            <div class="form-group">
              <label>邮箱</label>
              <input type="email" v-model="form.email" required placeholder="user@example.com" />
            </div>
            <button type="submit" class="btn btn-primary" :disabled="adding">
              {{ adding ? '添加中...' : '添加管理员' }}
            </button>
          </div>
        </form>
        <p v-if="result" class="result-text" :class="{ error: isError }">{{ result }}</p>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref } from 'vue'
import { useRouter } from 'vue-router'
import { useUserStore } from '../../stores/user'
import { adminAPI } from '../../api'

const router = useRouter()
const userStore = useUserStore()
const form = ref({ username: '', password: '', email: '' })
const adding = ref(false)
const result = ref('')
const isError = ref(false)

async function handleAdd() {
  adding.value = true; result.value = ''; isError.value = false
  try {
    const r = await adminAPI.addAdmin(form.value)
    result.value = `管理员 ${r.username} 创建成功 (ID: ${r.admin_id})`
    form.value = { username: '', password: '', email: '' }
  } catch (e) {
    result.value = '创建失败: ' + e.message; isError.value = true
  } finally { adding.value = false }
}

function handleLogout() { userStore.logout(); router.push('/admin/login') }
</script>

<style scoped>
.admin-page { display: flex; min-height: 100vh; background: #f1f5f9; }
.admin-sidebar { width: 220px; background: #0f172a; color: white; display: flex; flex-direction: column; position: fixed; height: 100vh; }
.sidebar-logo { padding: 1.5rem; font-weight: 700; font-size: 1rem; border-bottom: 1px solid #1e293b; }
.sidebar-nav { flex: 1; padding: 1rem 0; }
.nav-item { display: block; padding: 0.75rem 1.5rem; color: #94a3b8; text-decoration: none; font-size: 0.9375rem; border-left: 3px solid transparent; }
.nav-item:hover { color: white; background: rgba(255,255,255,0.05); }
.nav-item.active { color: white; background: rgba(14,165,233,0.15); border-left-color: #0ea5e9; }
.sidebar-footer { padding: 1rem 0; border-top: 1px solid #1e293b; }
.logout-btn { background: none; border: none; cursor: pointer; width: 100%; text-align: left; color: #94a3b8; font-family: inherit; font-size: 0.9375rem; padding: 0.75rem 1.5rem; }
.logout-btn:hover { color: white; }
.admin-content { flex: 1; margin-left: 220px; padding: 2rem; }
.admin-header h2 { font-size: 1.5rem; font-weight: 700; color: #1f2937; margin-bottom: 1.5rem; }
.card { background: white; border-radius: 12px; padding: 1.5rem; box-shadow: 0 1px 3px rgba(0,0,0,0.08); }
.card h3 { font-size: 1.125rem; margin-bottom: 1rem; }
.form-row { display: flex; gap: 1rem; align-items: flex-end; flex-wrap: wrap; }
.form-group { display: flex; flex-direction: column; gap: 0.25rem; }
.form-group label { font-size: 0.8125rem; color: #6b7280; }
.form-group input { padding: 0.5rem 0.75rem; border: 1px solid #d1d5db; border-radius: 8px; font-size: 0.9375rem; }
.add-form .btn { flex-shrink: 0; height: 38px; }
.result-text { margin-top: 1rem; color: #059669; font-size: 0.875rem; }
.result-text.error { color: #ef4444; }
</style>
