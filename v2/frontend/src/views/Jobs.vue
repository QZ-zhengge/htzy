<template>
  <div class="jobs-page">
    <div class="container">
      <h1 class="page-title">岗位列表</h1>
      
      <!-- 搜索和筛选 -->
      <div class="filter-section">
        <div class="search-box">
          <svg class="search-icon" viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
            <path d="M21 21l-6-6m2-5a7 7 0 11-14 0 7 7 0 0114 0z" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"/>
          </svg>
          <input 
            type="text" 
            class="form-input" 
            v-model="searchQuery"
            placeholder="搜索岗位名称..."
            @input="handleSearch"
          />
        </div>
        
        <div class="filter-group">
          <select class="form-select" v-model="selectedIndustry" @change="handleFilter">
            <option value="">所有行业</option>
            <option v-for="industry in industries" :key="industry" :value="industry">
              {{ industry }}
            </option>
          </select>
          
          <select class="form-select" v-model="selectedCity" @change="handleFilter">
            <option value="">所有城市</option>
            <option v-for="city in cities" :key="city" :value="city">
              {{ city }}
            </option>
          </select>
        </div>
      </div>

      <!-- 岗位列表 -->
      <div class="jobs-grid">
        <div v-if="loading" class="loading-state">
          <div class="loading-spinner"></div>
          <p>加载中...</p>
        </div>
        
        <div v-else-if="jobs.length === 0" class="empty-state">
          <p>未找到匹配的岗位</p>
        </div>
        
        <div v-else class="job-list">
          <div class="job-card" v-for="job in jobs" :key="job.id" @click="viewDetail(job)">
            <div class="job-header">
              <h3 class="job-title">{{ job.job_title }}</h3>
              <span class="job-salary">{{ job.salary_text || '面议' }}</span>
            </div>
            <div class="job-company">{{ job.company_name }}</div>
            <div class="job-meta">
              <span class="job-location">
                <svg viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
                  <path d="M17.657 16.657L13.414 20.9a1.998 1.998 0 01-2.827 0l-4.244-4.243a8 8 0 1111.314 0z" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"/>
                  <path d="M15 11a3 3 0 11-6 0 3 3 0 016 0z" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"/>
                </svg>
                {{ job.city || job.location }}
              </span>
              <span class="job-industry">{{ job.industry }}</span>
            </div>
            <div class="job-tags">
              <span class="tag" v-for="(tag, i) in job.tags?.slice(0, 3)" :key="i">{{ tag }}</span>
            </div>
          </div>
        </div>
      </div>

      <!-- 分页 -->
      <div v-if="totalPages > 1" class="pagination">
        <button 
          class="btn btn-ghost" 
          :disabled="currentPage === 1"
          @click="changePage(currentPage - 1)"
        >
          上一页
        </button>
        <span class="page-info">{{ currentPage }} / {{ totalPages }}</span>
        <button 
          class="btn btn-ghost" 
          :disabled="currentPage === totalPages"
          @click="changePage(currentPage + 1)"
        >
          下一页
        </button>
      </div>
    </div>

    <!-- 详情弹窗 -->
    <div v-if="showModal" class="modal-overlay" @click.self="showModal = false">
      <div class="modal modal-lg">
        <div class="modal-header">
          <h3 class="modal-title">{{ currentJob?.title }}</h3>
          <button class="modal-close" @click="showModal = false">&times;</button>
        </div>
        <div class="modal-body">
          <div v-if="currentJob" class="job-detail">
            <div class="detail-row">
              <span class="detail-label">公司：</span>
              <span class="detail-value">{{ currentJob.company_name }}</span>
            </div>
            <div class="detail-row">
              <span class="detail-label">薪资：</span>
              <span class="detail-value salary">{{ currentJob.salary_text || '面议' }}</span>
            </div>
            <div class="detail-row">
              <span class="detail-label">城市：</span>
              <span class="detail-value">{{ currentJob.city || currentJob.location }}</span>
            </div>
            <div class="detail-row">
              <span class="detail-label">行业：</span>
              <span class="detail-value">{{ currentJob.industry }}</span>
            </div>
            <div class="detail-section">
              <h4>岗位描述</h4>
              <p>{{ currentJob.job_desc || '暂无描述' }}</p>
            </div>
            <div class="detail-section">
              <h4>学历/经验要求</h4>
              <p>学历：{{ currentJob.education || '不限' }} | 经验：{{ currentJob.experience || '不限' }}</p>
            </div>
            <div class="detail-section" v-if="currentJob.job_tags">
              <div class="detail-tags">
                <span class="tag" v-for="(tag, i) in (Array.isArray(currentJob.job_tags) ? currentJob.job_tags : [])" :key="i">{{ tag }}</span>
              </div>
            </div>
          </div>
        </div>
        <div class="modal-footer">
          <button class="btn btn-ghost" @click="showModal = false">关闭</button>
          <button class="btn btn-primary" @click="createInterview">开始面试</button>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted, computed } from 'vue'
import { useRouter } from 'vue-router'
import { jobAPI, interviewAPI } from '../api'

const router = useRouter()

const jobs = ref([])
const loading = ref(true)
const searchQuery = ref('')
const selectedIndustry = ref('')
const selectedCity = ref('')
const currentPage = ref(1)
const pageSize = 12
const total = ref(0)

const showModal = ref(false)
const currentJob = ref(null)

const industries = ['互联网', '金融', '教育', '医疗', '制造', '电商']
const cities = ['北京', '上海', '广州', '深圳', '杭州', '成都']

const totalPages = computed(() => Math.ceil(total.value / pageSize))

onMounted(async () => {
  await loadJobs()
})

async function loadJobs() {
  loading.value = true
  
  try {
      const params = {
        page: currentPage.value,
        size: pageSize,
        keyword: searchQuery.value,
        industry: selectedIndustry.value,
        city: selectedCity.value
      }
      
      const result = await jobAPI.getList(params)
      jobs.value = result.data || []
      total.value = result.total || jobs.value.length
  } catch (err) {
    console.error('加载岗位列表失败:', err)
  } finally {
    loading.value = false
  }
}

function handleSearch() {
  currentPage.value = 1
  loadJobs()
}

function handleFilter() {
  currentPage.value = 1
  loadJobs()
}

function changePage(page) {
  currentPage.value = page
  loadJobs()
}

function viewDetail(job) {
  currentJob.value = job
  showModal.value = true
}

async function createInterview() {
  if (!currentJob.value) return
  
  try {
    const interview = await interviewAPI.create({
      job_position: currentJob.value.job_title
    })
    showModal.value = false
    router.push(`/interview/${interview.id}`)
  } catch (err) {
    alert('创建面试失败: ' + err.message)
  }
}
</script>

<style scoped>
.jobs-page {
  padding: 2rem 0;
}

.page-title {
  font-size: 1.75rem;
  font-weight: 700;
  color: var(--text-primary);
  margin-bottom: 2rem;
}

.filter-section {
  display: flex;
  gap: 1rem;
  margin-bottom: 2rem;
  flex-wrap: wrap;
}

.search-box {
  position: relative;
  flex: 1;
  min-width: 300px;
}

.search-icon {
  position: absolute;
  left: 1rem;
  top: 50%;
  transform: translateY(-50%);
  width: 20px;
  height: 20px;
  color: var(--text-secondary);
}

.search-box .form-input {
  padding-left: 3rem;
}

.filter-group {
  display: flex;
  gap: 0.75rem;
}

.filter-group .form-select {
  width: auto;
  min-width: 150px;
}

.jobs-grid {
  margin-bottom: 2rem;
}

.job-list {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(320px, 1fr));
  gap: 1.5rem;
}

.job-card {
  background: white;
  border-radius: 1rem;
  padding: 1.5rem;
  cursor: pointer;
  transition: var(--transition);
  border: 1px solid var(--border-color);
}

.job-card:hover {
  transform: translateY(-3px);
  box-shadow: var(--shadow-lg);
  border-color: var(--primary);
}

.job-header {
  display: flex;
  justify-content: space-between;
  align-items: flex-start;
  margin-bottom: 0.5rem;
}

.job-title {
  font-size: 1.125rem;
  font-weight: 600;
  color: var(--text-primary);
}

.job-salary {
  font-size: 1rem;
  font-weight: 600;
  color: var(--primary);
}

.job-company {
  font-size: 0.9375rem;
  color: var(--text-secondary);
  margin-bottom: 1rem;
}

.job-meta {
  display: flex;
  gap: 1rem;
  margin-bottom: 1rem;
}

.job-location,
.job-industry {
  display: flex;
  align-items: center;
  gap: 0.25rem;
  font-size: 0.875rem;
  color: var(--text-secondary);
}

.job-location svg {
  width: 16px;
  height: 16px;
}

.job-tags {
  display: flex;
  flex-wrap: wrap;
  gap: 0.5rem;
}

.tag {
  padding: 0.25rem 0.75rem;
  background: var(--background);
  color: var(--text-secondary);
  border-radius: 9999px;
  font-size: 0.75rem;
}

.loading-state,
.empty-state {
  text-align: center;
  padding: 4rem 2rem;
  background: white;
  border-radius: 1rem;
}

.loading-spinner {
  width: 48px;
  height: 48px;
  border: 4px solid var(--border-color);
  border-top-color: var(--primary);
  border-radius: 50%;
  animation: spin 1s linear infinite;
  margin: 0 auto 1rem;
}

@keyframes spin {
  to { transform: rotate(360deg); }
}

.empty-state p {
  color: var(--text-secondary);
}

.pagination {
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 1rem;
}

.page-info {
  font-size: 0.9375rem;
  color: var(--text-secondary);
}

.modal-lg {
  max-width: 600px;
}

.job-detail {
  display: flex;
  flex-direction: column;
  gap: 1rem;
}

.detail-row {
  display: flex;
  gap: 0.5rem;
}

.detail-label {
  font-weight: 500;
  color: var(--text-primary);
  min-width: 60px;
}

.detail-value {
  color: var(--text-secondary);
}

.detail-value.salary {
  color: var(--primary);
  font-weight: 600;
}

.detail-section {
  margin-top: 1rem;
}

.detail-section h4 {
  font-weight: 600;
  color: var(--text-primary);
  margin-bottom: 0.5rem;
}

.detail-section p {
  line-height: 1.8;
  color: var(--text-secondary);
}

.detail-tags {
  display: flex;
  flex-wrap: wrap;
  gap: 0.5rem;
  margin-top: 1rem;
}

@media (max-width: 768px) {
  .filter-section {
    flex-direction: column;
  }

  .filter-group {
    flex-direction: column;
  }

  .filter-group .form-select {
    width: 100%;
  }

  .job-list {
    grid-template-columns: 1fr;
  }
}
</style>