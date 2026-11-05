<template>
  <div class="admin-page">
    <div class="admin-sidebar">
      <div class="sidebar-logo">红途职引 · 管理后台</div>
      <nav class="sidebar-nav">
        <router-link to="/admin" class="nav-item" active-class="active">概览</router-link>
        <router-link to="/admin/students" class="nav-item" active-class="active">学生管理</router-link>
        <router-link to="/admin/scores" class="nav-item" active-class="active">成绩管理</router-link>
        <router-link v-if="userStore.isSuperAdmin" to="/admin/super" class="nav-item" active-class="active">管理员管理</router-link>
      </nav>
      <div class="sidebar-footer">
        <router-link to="/" class="nav-item">返回前台</router-link>
        <button class="nav-item logout-btn" @click="handleLogout">退出登录</button>
      </div>
    </div>
    <div class="admin-content">
      <div class="admin-header">
        <h2>系统概览</h2>
        <span class="admin-user">{{ userStore.username }}（{{ userStore.isSuperAdmin ? '超级管理员' : '管理员' }}）</span>
      </div>
      <div class="stats-grid">
        <div class="stat-card">
          <div class="stat-value">{{ stats.total_students }}</div>
          <div class="stat-label">花名册学生数</div>
        </div>
        <div class="stat-card">
          <div class="stat-value">{{ stats.registered }}</div>
          <div class="stat-label">已注册学生</div>
        </div>
        <div class="stat-card">
          <div class="stat-value">{{ stats.verified }}</div>
          <div class="stat-label">已验证学生</div>
        </div>
        <div class="stat-card">
          <div class="stat-value">{{ stats.completed_interviews }}</div>
          <div class="stat-label">完成面试数</div>
        </div>
        <div class="stat-card">
          <div class="stat-value">{{ stats.total_interviews }}</div>
          <div class="stat-label">总面试数</div>
        </div>
        <div class="stat-card">
          <div class="stat-value">{{ stats.average_score.toFixed(1) }}</div>
          <div class="stat-label">平均总分</div>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import { useRouter } from 'vue-router'
import { useUserStore } from '../../stores/user'
import { adminAPI } from '../../api'

const router = useRouter()
const userStore = useUserStore()
const stats = ref({ total_students: 0, registered: 0, verified: 0, total_interviews: 0, completed_interviews: 0, average_score: 0 })

onMounted(async () => {
  try { stats.value = await adminAPI.getStats() } catch {}
})

function handleLogout() {
  userStore.logout()
  router.push('/admin/login')
}
</script>

<style scoped>
.admin-page { display: flex; min-height: 100vh; background: #f1f5f9; }
.admin-sidebar { width: 220px; background: #0f172a; color: white; display: flex; flex-direction: column; position: fixed; height: 100vh; z-index: 10; }
.sidebar-logo { padding: 1.5rem; font-weight: 700; font-size: 1rem; border-bottom: 1px solid #1e293b; }
.sidebar-nav { flex: 1; padding: 1rem 0; }
.nav-item { display: block; padding: 0.75rem 1.5rem; color: #94a3b8; text-decoration: none; font-size: 0.9375rem; transition: all 0.2s; border-left: 3px solid transparent; }
.nav-item:hover { color: white; background: rgba(255,255,255,0.05); }
.nav-item.active { color: white; background: rgba(14,165,233,0.15); border-left-color: #0ea5e9; }
.sidebar-footer { padding: 1rem 0; border-top: 1px solid #1e293b; }
.logout-btn { background: none; border: none; cursor: pointer; width: 100%; text-align: left; font-family: inherit; }
.admin-content { flex: 1; margin-left: 220px; padding: 2rem; }
.admin-header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 2rem; }
.admin-header h2 { font-size: 1.5rem; font-weight: 700; color: #1f2937; }
.admin-user { color: #6b7280; font-size: 0.875rem; }
.stats-grid { display: grid; grid-template-columns: repeat(3, 1fr); gap: 1.5rem; }
.stat-card { background: white; border-radius: 12px; padding: 1.5rem; text-align: center; box-shadow: 0 1px 3px rgba(0,0,0,0.08); }
.stat-value { font-size: 2rem; font-weight: 700; color: #0ea5e9; }
.stat-label { font-size: 0.875rem; color: #6b7280; margin-top: 0.25rem; }
</style>
