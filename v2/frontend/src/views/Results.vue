<template>
  <div class="results-page">
    <div class="container">
      <div v-if="loading" class="loading-state">
        <div class="loading-spinner"></div>
        <p>加载中...</p>
      </div>
      
      <div v-else-if="results" class="results-content">
        <!-- 总分展示 -->
        <div class="score-card">
          <div class="score-ring">
            <svg viewBox="0 0 120 120">
              <circle 
                cx="60" 
                cy="60" 
                r="54" 
                fill="none" 
                stroke="#e5e7eb" 
                stroke-width="8"
              />
              <circle 
                cx="60" 
                cy="60" 
                r="54" 
                fill="none" 
                stroke="url(#gradient)" 
                stroke-width="8"
                stroke-linecap="round"
                :stroke-dasharray="339.292"
                :stroke-dashoffset="339.292 - (339.292 * results.overall_score / 100)"
                transform="rotate(-90 60 60)"
              />
              <defs>
                <linearGradient id="gradient" x1="0%" y1="0%" x2="100%" y2="100%">
                  <stop offset="0%" style="stop-color:#0ea5e9"/>
                  <stop offset="100%" style="stop-color:#06b6d4"/>
                </linearGradient>
              </defs>
            </svg>
            <div class="score-value">{{ results.overall_score }}</div>
          </div>
          <h2 class="score-title">面试总分</h2>
          <p class="score-desc">{{ getScoreDescription(results.overall_score) }}</p>
        </div>

        <!-- 三维评分 -->
        <div class="dimensions-card">
          <h3 class="card-title">多维评分</h3>
          <div class="dimensions-grid">
            <div class="dimension-item">
              <div class="dimension-label">专业能力</div>
              <div class="dimension-bar">
                <div class="dimension-fill" :style="{ width: results.professional_score + '%' }"></div>
              </div>
              <div class="dimension-value">{{ results.professional_score }}</div>
            </div>
            <div class="dimension-item">
              <div class="dimension-label">语言表达</div>
              <div class="dimension-bar">
                <div class="dimension-fill" :style="{ width: results.language_score + '%' }"></div>
              </div>
              <div class="dimension-value">{{ results.language_score }}</div>
            </div>
            <div class="dimension-item">
              <div class="dimension-label">表情管理</div>
              <div class="dimension-bar">
                <div class="dimension-fill" :style="{ width: results.expression_score + '%' }"></div>
              </div>
              <div class="dimension-value">{{ results.expression_score }}</div>
            </div>
          </div>
        </div>

        <!-- AI 报告 -->
        <div class="report-card">
          <h3 class="card-title">AI 评估报告</h3>
          <div class="report-content" v-if="parsedReport">
            <div v-if="parsedReport.summary" class="report-section">
              <h4>综合评价</h4>
              <p>{{ parsedReport.summary }}</p>
            </div>
            <div v-if="parsedReport.overall_evaluation" class="report-section">
              <h4>总体评价</h4>
              <p>{{ parsedReport.overall_evaluation }}</p>
            </div>
            <div v-if="parsedReport.strengths && parsedReport.strengths.length" class="report-section">
              <h4>优势</h4>
              <ul><li v-for="(s, i) in parsedReport.strengths" :key="i">{{ s }}</li></ul>
            </div>
            <div v-if="parsedReport.weaknesses && parsedReport.weaknesses.length" class="report-section">
              <h4>待改进</h4>
              <ul><li v-for="(w, i) in parsedReport.weaknesses" :key="i">{{ w }}</li></ul>
            </div>
            <div v-if="parsedReport.improvements && parsedReport.improvements.length" class="report-section">
              <h4>提升建议</h4>
              <ul><li v-for="(imp, i) in parsedReport.improvements" :key="i">{{ imp }}</li></ul>
            </div>
            <div v-if="parsedReport.ability_assessment && Object.keys(parsedReport.ability_assessment).length" class="report-section">
              <h4>能力评估</h4>
              <div v-for="(val, key) in parsedReport.ability_assessment" :key="key" class="ability-item">
                <span class="ability-label">{{ abilityLabel(key) }}：</span>
                <span class="ability-value">{{ val }}</span>
              </div>
            </div>
          </div>
          <div v-else class="report-content">
            <p>{{ results.report }}</p>
          </div>
        </div>

        <!-- 操作按钮 -->
        <div class="actions">
          <router-link to="/dashboard" class="btn btn-primary">返回仪表板</router-link>
          <router-link to="/jobs" class="btn btn-outline">继续面试</router-link>
        </div>
      </div>
      
      <div v-else class="empty-state">
        <p>未找到面试结果</p>
        <router-link to="/dashboard" class="btn btn-primary">返回仪表板</router-link>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, computed, onMounted } from 'vue'
import { useRoute } from 'vue-router'
import { interviewAPI } from '../api'

const route = useRoute()
const interviewId = route.params.id

const results = ref(null)
const loading = ref(true)

const parsedReport = computed(() => {
  if (!results.value?.report) return null
  try {
    const obj = JSON.parse(results.value.report)
    return (obj.summary || obj.overall_evaluation || obj.strengths || obj.weaknesses) ? obj : null
  } catch {
    return null
  }
})

const abilityLabels = {
  clarity: '清晰度', dressing: '着装整洁', eye_contact: '眼神交流',
  facial: '面部表情', mandarin: '普通话', organization: '语言组织',
  stress: '压力管理', professional_average: '专业平均', stability: '稳定性'
}

function abilityLabel(key) {
  return abilityLabels[key] || key
}

onMounted(async () => {
  await loadResults()
})

async function loadResults() {
  try {
    results.value = await interviewAPI.getResults(interviewId)
  } catch (err) {
    console.error('加载结果失败:', err)
  } finally {
    loading.value = false
  }
}

function getScoreDescription(score) {
  if (score >= 90) return '非常优秀！你的面试表现堪称完美。'
  if (score >= 80) return '表现优秀！你已经具备很强的竞争力。'
  if (score >= 70) return '表现良好，还有提升空间。'
  if (score >= 60) return '基本合格，建议加强薄弱环节。'
  return '需要改进，建议多加练习。'
}
</script>

<style scoped>
.results-page {
  padding: 2rem 0;
  min-height: calc(100vh - 64px);
  background: var(--background);
}

.loading-state {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  min-height: 50vh;
  gap: 1rem;
}

.loading-spinner {
  width: 48px;
  height: 48px;
  border: 4px solid var(--border-color);
  border-top-color: var(--primary);
  border-radius: 50%;
  animation: spin 1s linear infinite;
}

@keyframes spin {
  to { transform: rotate(360deg); }
}

.results-content {
  max-width: 800px;
  margin: 0 auto;
}

.score-card {
  background: white;
  border-radius: 1rem;
  padding: 3rem;
  text-align: center;
  box-shadow: var(--shadow-lg);
  margin-bottom: 1.5rem;
}

.score-ring {
  position: relative;
  width: 160px;
  height: 160px;
  margin: 0 auto 1.5rem;
}

.score-ring svg {
  width: 100%;
  height: 100%;
}

.score-value {
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translate(-50%, -50%);
  font-size: 3rem;
  font-weight: 700;
  background: var(--gradient);
  -webkit-background-clip: text;
  -webkit-text-fill-color: transparent;
  background-clip: text;
}

.score-title {
  font-size: 1.5rem;
  font-weight: 700;
  color: var(--text-primary);
  margin-bottom: 0.5rem;
}

.score-desc {
  font-size: 1rem;
  color: var(--text-secondary);
}

.dimensions-card,
.report-card {
  background: white;
  border-radius: 1rem;
  padding: 1.5rem;
  box-shadow: var(--shadow-md);
  margin-bottom: 1.5rem;
}

.card-title {
  font-size: 1.25rem;
  font-weight: 600;
  color: var(--text-primary);
  margin-bottom: 1.5rem;
}

.dimensions-grid {
  display: flex;
  flex-direction: column;
  gap: 1.5rem;
}

.dimension-item {
  display: grid;
  grid-template-columns: 100px 1fr 50px;
  align-items: center;
  gap: 1rem;
}

.dimension-label {
  font-weight: 500;
  color: var(--text-primary);
}

.dimension-bar {
  height: 12px;
  background: var(--background);
  border-radius: 6px;
  overflow: hidden;
}

.dimension-fill {
  height: 100%;
  background: var(--gradient);
  border-radius: 6px;
  transition: width 0.5s ease;
}

.dimension-value {
  font-weight: 600;
  color: var(--text-primary);
  text-align: right;
}

.report-content {
  line-height: 1.8;
  color: var(--text-primary);
}
.report-section {
  margin-bottom: 1.25rem;
  padding-bottom: 1.25rem;
  border-bottom: 1px solid #f1f5f9;
}
.report-section:last-child { border-bottom: none; margin-bottom: 0; padding-bottom: 0; }
.report-section h4 {
  font-size: 1rem;
  font-weight: 600;
  color: #1e293b;
  margin-bottom: 0.5rem;
}
.report-section p { color: #475569; }
.report-section ul {
  list-style: none;
  padding: 0;
}
.report-section li {
  position: relative;
  padding-left: 1.25rem;
  margin-bottom: 0.375rem;
  color: #475569;
}
.report-section li::before {
  content: '•';
  position: absolute;
  left: 0;
  color: #0ea5e9;
  font-weight: 700;
}
.ability-item {
  display: flex;
  gap: 0.5rem;
  margin-bottom: 0.25rem;
  font-size: 0.9375rem;
}
.ability-label { color: #64748b; }
.ability-value { color: #1e293b; font-weight: 500; }

.actions {
  display: flex;
  gap: 1rem;
  justify-content: center;
}

.empty-state {
  text-align: center;
  padding: 4rem 2rem;
  background: white;
  border-radius: 1rem;
}

.empty-state p {
  color: var(--text-secondary);
  margin-bottom: 1rem;
}

@media (max-width: 640px) {
  .dimension-item {
    grid-template-columns: 80px 1fr 40px;
  }

  .actions {
    flex-direction: column;
  }
}
</style>