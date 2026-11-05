<template>
  <div class="delivery-page">
    <div class="page-header">
      <h1>自动投递</h1>
      <p>系统每天为你自动匹配并投递最合适的 5 个岗位</p>
    </div>

    <!-- Config Card -->
    <div class="card">
      <h3>投递设置</h3>
      <div class="config-form">
        <div class="form-group">
          <label>投递简历</label>
          <select v-model="config.resume_id" class="form-input">
            <option value="0">请选择简历</option>
            <option v-for="r in resumes" :key="r.id" :value="r.id">{{ r.file_path.split('/').pop() }}</option>
          </select>
        </div>
        <div class="form-group">
          <label>期望岗位关键词</label>
          <input type="text" class="form-input" v-model="config.job_keywords" placeholder="例：前端开发、Java" />
        </div>
        <div class="form-group">
          <label>期望城市</label>
          <input type="text" class="form-input" v-model="config.location_pref" placeholder="例：长春、北京" />
        </div>
        <div class="toggle-row">
          <label class="toggle">
            <input type="checkbox" v-model="config.enabled" @change="saveConfig" />
            <span class="toggle-slider"></span>
          </label>
          <span class="toggle-label">{{ config.enabled ? '自动投递已开启' : '自动投递已关闭' }}</span>
        </div>
      </div>
    </div>

    <!-- Suggested Jobs -->
    <div class="card" v-if="suggestedJobs.length">
      <h3>系统推荐岗位</h3>
      <div class="job-list">
        <div v-for="job in suggestedJobs" :key="job.id" class="job-item">
          <div class="job-info">
            <h4>{{ job.job_title }}</h4>
            <p>{{ job.company_name }} · {{ job.city || '未知' }} · {{ job.salary_text || '面议' }}</p>
          </div>
          <span class="badge badge-primary">匹配</span>
        </div>
      </div>
    </div>

    <!-- Delivery Logs -->
    <div class="card">
      <h3>投递记录</h3>
      <div v-if="logs.length" class="log-list">
        <div v-for="log in logs" :key="log.id" class="log-item">
          <div class="log-info">
            <strong>{{ log.company_name }}</strong> - {{ log.job_title }}
            <span class="log-time">{{ formatDate(log.sent_at) }}</span>
          </div>
          <span :class="log.status === 'sent' ? 'badge badge-success' : 'badge badge-warning'">
            {{ log.status === 'sent' ? '已投递' : '投递失败' }}
          </span>
        </div>
      </div>
      <p v-else class="empty">暂无投递记录</p>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import { deliveryAPI, resumeAPI } from '../api'

const config = ref({ enabled: false, resume_id: 0, job_keywords: '', location_pref: '' })
const resumes = ref([])
const suggestedJobs = ref([])
const logs = ref([])

onMounted(async () => {
  try {
    const r = await resumeAPI.getList()
    resumes.value = Array.isArray(r) ? r : (r?.list || [])
  } catch {}
  try { config.value = await deliveryAPI.getConfig() } catch {}
  try { suggestedJobs.value = (await deliveryAPI.getSuggested()).jobs || [] } catch {}
  try { logs.value = await deliveryAPI.getLogs() } catch {}
})

async function saveConfig() {
  try {
    await deliveryAPI.toggle(config.value)
    if (config.value.enabled) {
      try { suggestedJobs.value = (await deliveryAPI.getSuggested()).jobs || [] } catch {}
    }
  } catch (e) { alert('保存失败: ' + e.message) }
}

function formatDate(d) { return d ? new Date(d).toLocaleString('zh-CN') : '-' }
</script>

<style scoped>
.delivery-page { max-width: 800px; margin: 0 auto; padding: 2rem 1rem; }
.page-header { margin-bottom: 2rem; }
.page-header h1 { font-size: 1.75rem; font-weight: 700; color: var(--text-primary); }
.page-header p { color: var(--text-secondary); margin-top: 0.25rem; }
.card { background: white; border-radius: 12px; padding: 1.5rem; margin-bottom: 1.5rem; box-shadow: 0 1px 3px rgba(0,0,0,0.08); }
.card h3 { font-size: 1.125rem; font-weight: 600; margin-bottom: 1rem; }
.config-form { display: flex; flex-direction: column; gap: 1rem; }
.form-group { display: flex; flex-direction: column; gap: 0.25rem; }
.form-group label { font-size: 0.875rem; font-weight: 500; color: #374151; }
.form-input { padding: 0.5rem 0.75rem; border: 1px solid #d1d5db; border-radius: 8px; font-size: 0.9375rem; }
.toggle-row { display: flex; align-items: center; gap: 0.75rem; margin-top: 0.5rem; }
.toggle { position: relative; display: inline-block; width: 48px; height: 26px; cursor: pointer; }
.toggle input { opacity: 0; width: 0; height: 0; }
.toggle-slider { position: absolute; inset: 0; background: #d1d5db; border-radius: 13px; transition: 0.3s; }
.toggle-slider::before { content: ''; position: absolute; width: 20px; height: 20px; left: 3px; bottom: 3px; background: white; border-radius: 50%; transition: 0.3s; }
.toggle input:checked + .toggle-slider { background: #0ea5e9; }
.toggle input:checked + .toggle-slider::before { transform: translateX(22px); }
.toggle-label { font-size: 0.9375rem; font-weight: 500; color: #374151; }
.job-list, .log-list { display: flex; flex-direction: column; gap: 0.75rem; }
.job-item, .log-item { display: flex; justify-content: space-between; align-items: center; padding: 0.75rem; border: 1px solid #f1f5f9; border-radius: 8px; }
.job-info h4 { font-size: 0.9375rem; color: #1f2937; }
.job-info p, .log-info { font-size: 0.8125rem; color: #6b7280; }
.log-info { display: flex; flex-direction: column; gap: 0.125rem; }
.log-time { font-size: 0.75rem; color: #9ca3af; }
.badge { padding: 0.125rem 0.5rem; border-radius: 9999px; font-size: 0.75rem; }
.badge-primary { background: #e0f2fe; color: #0369a1; }
.badge-success { background: #dcfce7; color: #166534; }
.badge-warning { background: #fef9c3; color: #854d0e; }
.empty { color: #9ca3af; text-align: center; padding: 1.5rem; }
</style>
