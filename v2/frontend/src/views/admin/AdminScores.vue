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
        <h2>成绩管理</h2>
        <button class="btn btn-primary" @click="exportCSV">导出 Excel (CSV)</button>
      </div>

      <div class="table-card">
        <table class="data-table">
          <thead>
            <tr>
              <th>学号</th><th>姓名</th><th>班级</th><th>岗位</th><th>状态</th>
              <th>专业</th><th>语言</th><th>表情</th><th>总分</th><th>时间</th>
            </tr>
          </thead>
          <tbody>
            <tr v-for="(s, i) in scores" :key="i">
              <td>{{ s.student_id || '-' }}</td>
              <td>{{ s.real_name || '-' }}</td>
              <td>{{ s.class_name || '-' }}</td>
              <td>{{ s.job_position }}</td>
              <td><span :class="s.status === 'completed' ? 'badge-green' : 'badge-gray'">{{ statusText(s.status) }}</span></td>
              <td>{{ s.professional_score || '-' }}</td>
              <td>{{ s.language_score || '-' }}</td>
              <td>{{ s.expression_score || '-' }}</td>
              <td :class="{ 'score-highlight': s.overall_score }">{{ s.overall_score || '-' }}</td>
              <td>{{ formatDate(s.created_at) }}</td>
            </tr>
          </tbody>
        </table>
        <p v-if="scores.length === 0" class="empty">暂无面试成绩数据</p>
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
const scores = ref([])

onMounted(async () => {
  try { scores.value = await adminAPI.getScores() } catch {}
})

function exportCSV() {
  // Build CSV client-side
  const headers = ['学号','姓名','班级','用户名','岗位','时间','专业能力','语言表达','表情管理','总分']
  const rows = scores.value.map(s => [
    s.student_id, s.real_name, s.class_name, s.username, s.job_position,
    s.created_at, s.professional_score, s.language_score, s.expression_score, s.overall_score
  ])
  const bom = '\uFEFF'
  const csv = bom + [headers, ...rows].map(r => r.join(',')).join('\n')
  const blob = new Blob([csv], { type: 'text/csv;charset=utf-8' })
  const url = URL.createObjectURL(blob)
  const a = document.createElement('a')
  a.href = url; a.download = 'htzy_scores.csv'; a.click()
  URL.revokeObjectURL(url)
}

function formatDate(d) { return d ? new Date(d).toLocaleDateString('zh-CN') : '-' }
function statusText(s) { return { pending: '待开始', in_progress: '进行中', completed: '已完成' }[s] || s }
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
.admin-header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 1.5rem; }
.admin-header h2 { font-size: 1.5rem; font-weight: 700; color: #1f2937; }
.table-card { background: white; border-radius: 12px; padding: 1.5rem; box-shadow: 0 1px 3px rgba(0,0,0,0.08); overflow-x: auto; }
.data-table { width: 100%; border-collapse: collapse; }
.data-table th, .data-table td { padding: 0.625rem 0.75rem; text-align: left; border-bottom: 1px solid #f1f5f9; font-size: 0.8125rem; white-space: nowrap; }
.data-table th { color: #6b7280; font-weight: 600; background: #f8fafc; }
.badge-green { background: #dcfce7; color: #166534; padding: 0.125rem 0.5rem; border-radius: 9999px; font-size: 0.75rem; }
.badge-gray { background: #f1f5f9; color: #64748b; padding: 0.125rem 0.5rem; border-radius: 9999px; font-size: 0.75rem; }
.score-highlight { font-weight: 700; color: #0ea5e9; }
.empty { color: #9ca3af; text-align: center; padding: 2rem; }
</style>
