<template>
  <div class="dashboard">
    <div class="container">
      <!-- 欢迎语 -->
      <div class="welcome-section">
        <h1 class="welcome-title">欢迎回来，{{ username }}！</h1>
        <p class="welcome-subtitle">这是你的面试数据概览</p>
      </div>

      <!-- 统计卡片 -->
      <div class="stats-grid">
        <div class="stat-card">
          <div class="stat-icon completed">
            <svg viewBox="0 0 24 24" fill="none"><path d="M9 12l2 2 4-4m6 2a9 9 0 11-18 0 9 9 0 0118 0z" stroke="currentColor" stroke-width="2"/></svg>
          </div>
          <div class="stat-info">
            <div class="stat-value">{{ stats.completed }}</div>
            <div class="stat-label">已完成面试</div>
          </div>
        </div>
        <div class="stat-card">
          <div class="stat-icon score">
            <svg viewBox="0 0 24 24" fill="none"><path d="M11.049 2.927c.3-.921 1.603-.921 1.902 0l1.519 4.674a1 1 0 00.95.69h4.915c.969 0 1.371 1.24.588 1.81l-3.976 2.888a1 1 0 00-.363 1.118l1.518 4.674c.3.922-.755 1.688-1.538 1.118l-3.976-2.888a1 1 0 00-1.176 0l-3.976 2.888c-.783.57-1.838-.197-1.538-1.118l1.518-4.674a1 1 0 00-.363-1.118l-3.976-2.888c-.784-.57-.38-1.81.588-1.81h4.914a1 1 0 00.951-.69l1.519-4.674z" stroke="currentColor" stroke-width="2"/></svg>
          </div>
          <div class="stat-info">
            <div class="stat-value">{{ stats.averageScore }}</div>
            <div class="stat-label">平均分数</div>
          </div>
        </div>
        <div class="stat-card">
          <div class="stat-icon pending">
            <svg viewBox="0 0 24 24" fill="none"><path d="M12 8v4l3 3m6-3a9 9 0 11-18 0 9 9 0 0118 0z" stroke="currentColor" stroke-width="2"/></svg>
          </div>
          <div class="stat-info">
            <div class="stat-value">{{ stats.pending }}</div>
            <div class="stat-label">待完成面试</div>
          </div>
        </div>
      </div>

      <!-- 最近面试结果 -->
      <div class="card">
        <div class="card-header"><h2 class="card-title">最近面试结果</h2></div>
        <div class="card-body">
          <div v-if="recentInterviews.length === 0" class="empty-state">
            <p>暂无面试记录</p>
            <router-link to="/jobs" class="btn btn-primary">浏览岗位开始面试</router-link>
          </div>
          <div v-else class="table-container">
            <table class="table">
              <thead>
                <tr><th>岗位</th><th>日期</th><th>分数</th><th>状态</th><th>操作</th></tr>
              </thead>
              <tbody>
                <tr v-for="i in recentInterviews" :key="i.id">
                  <td>{{ i.job_position }}</td>
                  <td>{{ formatDate(i.created_at) }}</td>
                  <td>
                    <span class="score-badge" :class="getScoreClass(i.overall_score)">
                      {{ i.overall_score != null ? Number(i.overall_score).toFixed(1) : '-' }}
                    </span>
                  </td>
                  <td><span class="badge" :class="getStatusClass(i.status)">{{ getStatusText(i.status) }}</span></td>
                  <td>
                    <router-link v-if="i.status==='completed'" :to="`/results/${i.id}`" class="btn btn-sm btn-outline">查看报告</router-link>
                    <router-link v-else-if="i.status==='pending'" :to="`/interview/${i.id}`" class="btn btn-sm btn-primary">开始面试</router-link>
                    <router-link v-else-if="i.status==='in_progress'" :to="`/interview/${i.id}`" class="btn btn-sm btn-primary">继续面试</router-link>
                  </td>
                </tr>
              </tbody>
            </table>
          </div>
        </div>
      </div>

      <!-- 创建新面试 -->
      <div class="card">
        <div class="card-header"><h2 class="card-title">创建新面试</h2></div>
        <div class="card-body">
          <form class="create-form" @submit.prevent="handleCreateInterview">
            <div class="form-group" style="flex:1">
              <label class="form-label">目标岗位名称</label>
              <div class="input-button-row">
                <input type="text" class="form-input" v-model="jobPosition" placeholder="例：后端开发工程师" required />
                <button type="submit" class="btn btn-primary" :disabled="creating">
                  {{ creating ? '创建中...' : '创建面试' }}
                </button>
              </div>
            </div>
          </form>
        </div>
      </div>
    </div>

    <!-- Loading Modal -->
    <div v-if="showLoading" class="loading-overlay">
      <div class="loading-modal">
        <div class="loading-spinner-large"></div>
        <h3 class="loading-title">AI 正在努力工作中...</h3>
        <p class="loading-subtitle">正在为你的面试生成个性化题目，请稍候</p>
        <div class="loading-tip">
          <span class="tip-icon">💡</span>
          <span class="tip-text">{{ loadingTip }}</span>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted, computed } from 'vue'
import { useRouter } from 'vue-router'
import { interviewAPI, resumeAPI } from '../api'
import { useUserStore } from '../stores/user'

const router = useRouter()
const userStore = useUserStore()
const username = computed(() => userStore.username)

const stats = ref({ completed: 0, averageScore: 0, pending: 0 })
const recentInterviews = ref([])
const jobPosition = ref('')
const creating = ref(false)
const showLoading = ref(false)
const loadingTip = ref('')

onMounted(() => loadData())

async function loadData() {
  try {
    const interviews = await interviewAPI.getList()
    recentInterviews.value = interviews.slice(0, 5)

    const completed = interviews.filter(i => i.status === 'completed')
    stats.value.completed = completed.length
    stats.value.averageScore = completed.length > 0
      ? Math.round(completed.reduce((s, i) => s + (parseFloat(i.overall_score) || 0), 0) / completed.length)
      : 0
    stats.value.pending = interviews.filter(i => i.status === 'pending' || i.status === 'in_progress').length
  } catch (err) {
    console.error('Load data failed:', err)
  }
}

async function handleCreateInterview() {
  if (!jobPosition.value.trim()) {
    alert('请输入岗位名称')
    return
  }

  // Check if user has uploaded at least one resume
  try {
    const resumeList = await resumeAPI.getList()
    const list = Array.isArray(resumeList) ? resumeList : (resumeList?.list || [])
    if (list.length === 0) {
      alert('请先上传简历，然后再创建面试！')
      router.push('/resume')
      return
    }
  } catch {
    alert('请先上传简历，然后再创建面试！')
    router.push('/resume')
    return
  }

  creating.value = true
  showLoading.value = true
  loadingTip.value = getRandomTip()
  try {
    const result = await interviewAPI.create({ job_position: jobPosition.value.trim() })
    showLoading.value = false
    router.push(`/interview/${result.interview_id}`)
  } catch (err) {
    showLoading.value = false
    alert('创建面试失败: ' + err.message)
  } finally {
    creating.value = false
  }
}

function formatDate(d) { return d ? new Date(d).toLocaleDateString('zh-CN') : '-' }
function getScoreClass(s) { const n = parseFloat(s); if (n >= 80) return 'score-high'; if (n >= 60) return 'score-medium'; return s ? 'score-low' : '' }
function getStatusClass(s) { return { pending:'badge-warning', in_progress:'badge-primary', completed:'badge-success' }[s] || 'badge-secondary' }
function getStatusText(s) { return { pending:'待开始', in_progress:'进行中', completed:'已完成' }[s] || s }

const interviewTips = [
  '面试前多喝点水，防止口渴影响发挥哦~',
  '提前10分钟到达面试地点，给自己一个缓冲的时间。',
  '坐姿挺拔、目光自然交流，自信是最好的名片。',
  '回答问题时可以稍作停顿思考，比脱口而出更有深度。',
  '用STAR法则回答行为面试题：情境→任务→行动→结果。',
  '准备2-3个反问面试官的问题，显示你的主动性。',
  '面试前对着镜子练习微笑，自然的表情能缓解紧张。',
  '适当点头回应面试官，表示你在认真倾听。',
  '语速放慢一点，清晰比快速更重要。',
  '面试结束后记得发一封感谢邮件，给面试官留下好印象。',
  '深呼吸三次再开始，让心率降下来效果更好。',
  '聊项目时多用数字量化成果，比如提升了30%效率。',
]
function getRandomTip() {
  return interviewTips[Math.floor(Math.random() * interviewTips.length)]
}
</script>

<style scoped>
.dashboard { padding: 2rem 0; }
.welcome-section { margin-bottom: 2rem; }
.welcome-title { font-size: 1.75rem; font-weight: 700; color: var(--text-primary); margin-bottom: 0.5rem; }
.welcome-subtitle { font-size: 1rem; color: var(--text-secondary); }
.stats-grid { display: grid; grid-template-columns: repeat(3, 1fr); gap: 1.5rem; margin-bottom: 2rem; }
.stat-card { background: white; border-radius: 1rem; padding: 1.5rem; display: flex; align-items: center; gap: 1rem; box-shadow: var(--shadow-md); transition: var(--transition); }
.stat-card:hover { transform: translateY(-2px); box-shadow: var(--shadow-lg); }
.stat-icon { width: 56px; height: 56px; border-radius: 12px; display: flex; align-items: center; justify-content: center; }
.stat-icon svg { width: 28px; height: 28px; }
.stat-icon.completed { background-color: rgba(16,185,129,0.1); color: var(--success); }
.stat-icon.score { background-color: rgba(14,165,233,0.1); color: var(--primary); }
.stat-icon.pending { background-color: rgba(245,158,11,0.1); color: var(--warning); }
.stat-value { font-size: 2rem; font-weight: 700; color: var(--text-primary); }
.stat-label { font-size: 0.875rem; color: var(--text-secondary); margin-top: 0.25rem; }
.card { margin-bottom: 1.5rem; }
.empty-state { text-align: center; padding: 3rem; }
.empty-state p { color: var(--text-secondary); margin-bottom: 1rem; }
.score-badge { display: inline-block; padding: 0.25rem 0.75rem; border-radius: 9999px; font-weight: 600; font-size: 0.875rem; }
.score-high { background-color: rgba(16,185,129,0.1); color: var(--success); }
.score-medium { background-color: rgba(245,158,11,0.1); color: var(--warning); }
.score-low { background-color: rgba(239,68,68,0.1); color: var(--danger); }
.create-form { display: flex; flex-direction: column; gap: 1rem; }
.input-button-row { display: flex; gap: 0.75rem; align-items: flex-end; }
.input-button-row .form-input { flex: 1; margin-bottom: 0; }
.input-button-row .btn { flex-shrink: 0; height: 40px; white-space: nowrap; }
.form-row { flex: 1; }
@media (max-width: 768px) { .stats-grid { grid-template-columns: 1fr; } .create-form { flex-direction: column; } .input-button-row { flex-direction: column; } }

/* Loading Modal */
.loading-overlay {
  position: fixed; inset: 0; background: rgba(0,0,0,0.5);
  display: flex; align-items: center; justify-content: center;
  z-index: 9999;
}
.loading-modal {
  background: white; border-radius: 16px; padding: 3rem 2.5rem;
  text-align: center; max-width: 420px; width: 90%;
  box-shadow: 0 20px 60px rgba(0,0,0,0.2);
}
.loading-spinner-large {
  width: 48px; height: 48px; border: 4px solid #e5e7eb;
  border-top-color: #0ea5e9; border-radius: 50%;
  animation: spin 0.8s linear infinite; margin: 0 auto 1.5rem;
}
@keyframes spin { to { transform: rotate(360deg); } }
.loading-title { font-size: 1.25rem; font-weight: 700; color: #1f2937; margin-bottom: 0.5rem; }
.loading-subtitle { font-size: 0.9375rem; color: #6b7280; margin-bottom: 1.5rem; }
.loading-tip {
  background: #fffbeb; border: 1px solid #fde68a; border-radius: 10px;
  padding: 1rem 1.25rem; display: flex; align-items: flex-start; gap: 0.75rem;
  text-align: left;
}
.tip-icon { font-size: 1.25rem; flex-shrink: 0; margin-top: 0.1rem; }
.tip-text { font-size: 0.875rem; color: #92400e; line-height: 1.5; }
</style>
