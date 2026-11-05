<template>
  <div class="detail-page">
    <div class="container" v-if="story">
      <!-- Back button -->
      <button class="back-btn" @click="$router.push('/red-guide')">← 返回列表</button>

      <!-- Header -->
      <div class="detail-header">
        <span class="detail-category">{{ getCategoryLabel(story.category) }}</span>
        <h1 class="detail-title">{{ story.title }}</h1>
        <div class="detail-meta">
          <span>来源：{{ story.source_name }}</span>
          <span>阅读：{{ story.view_count }}</span>
        </div>
      </div>

      <!-- Content -->
      <div class="detail-body">
        <div class="content-paragraphs">
          <p v-for="(para, idx) in paragraphs" :key="idx" class="content-para">{{ para }}</p>
        </div>
      </div>

      <!-- Source link -->
      <div class="detail-source" v-if="story.source_url">
        <p>📖 原文链接：</p>
        <a :href="story.source_url" target="_blank" rel="noopener" class="source-link">
          {{ story.source_name }} — {{ story.source_url }}
        </a>
      </div>

      <!-- Back -->
      <div class="detail-footer">
        <button class="back-btn-bottom" @click="$router.push('/red-guide')">← 返回红色指引</button>
      </div>
    </div>

    <div v-else-if="loading" class="loading-state">
      <div class="spinner"></div>
      <p>加载中...</p>
    </div>
  </div>
</template>

<script setup>
import { ref, computed, onMounted } from 'vue'
import { useRoute } from 'vue-router'
import { redStoryAPI } from '../api'

const route = useRoute()
const story = ref(null)
const loading = ref(true)

const categoryMap = {
  computer_pioneer: '计算机先驱',
  chip_pioneer: '芯片之光',
  two_bombs_one_satellite: '两弹一星'
}

function getCategoryLabel(cat) {
  return categoryMap[cat] || cat
}

const paragraphs = computed(() => {
  if (!story.value?.content) return []
  // Split content by Chinese period or newline
  return story.value.content
    .split(/(?<=[。！？\n])\s*/)
    .map(p => p.trim())
    .filter(p => p.length > 0)
})

onMounted(async () => {
  try {
    story.value = await redStoryAPI.getDetail(route.params.id)
  } catch (err) {
    console.error('Load story detail failed:', err)
  } finally {
    loading.value = false
  }
})
</script>

<style scoped>
.detail-page {
  min-height: 100vh;
  background: #f8fafc;
  padding: 2rem 0 4rem;
}

.back-btn {
  display: inline-flex;
  align-items: center;
  padding: 0.5rem 1rem;
  border: 1px solid #d1d5db;
  border-radius: 8px;
  background: white;
  color: #374151;
  font-size: 0.875rem;
  cursor: pointer;
  transition: all 0.2s;
  margin-bottom: 1.5rem;
}
.back-btn:hover { border-color: #dc2626; color: #dc2626; }

.detail-header {
  background: white;
  border-radius: 16px;
  padding: 2.5rem;
  box-shadow: 0 1px 3px rgba(0,0,0,0.06);
  margin-bottom: 2rem;
  border-left: 5px solid #dc2626;
}

.detail-category {
  display: inline-block;
  padding: 0.25rem 0.75rem;
  border-radius: 9999px;
  font-size: 0.75rem;
  font-weight: 600;
  background: #fef2f2;
  color: #dc2626;
  margin-bottom: 1rem;
}

.detail-title {
  font-size: 1.75rem;
  font-weight: 800;
  color: #1f2937;
  line-height: 1.4;
  margin-bottom: 1rem;
}

.detail-meta {
  display: flex;
  gap: 2rem;
  font-size: 0.875rem;
  color: #9ca3af;
}

.detail-body {
  background: white;
  border-radius: 16px;
  padding: 2.5rem;
  box-shadow: 0 1px 3px rgba(0,0,0,0.06);
  margin-bottom: 2rem;
}

.content-para {
  font-size: 1rem;
  line-height: 2;
  color: #374151;
  margin-bottom: 1.25rem;
  text-indent: 2em;
}
.content-para:last-child { margin-bottom: 0; }

.detail-source {
  background: #fffbeb;
  border: 1px solid #fde68a;
  border-radius: 12px;
  padding: 1.5rem;
  margin-bottom: 2rem;
}
.detail-source p {
  font-size: 0.875rem;
  color: #92400e;
  margin-bottom: 0.5rem;
  font-weight: 600;
}
.source-link {
  font-size: 0.8125rem;
  color: #2563eb;
  word-break: break-all;
  text-decoration: none;
}
.source-link:hover { text-decoration: underline; }

.detail-footer {
  text-align: center;
  padding: 1rem 0;
}
.back-btn-bottom {
  padding: 0.75rem 2rem;
  border-radius: 9999px;
  background: #dc2626;
  color: white;
  border: none;
  font-size: 1rem;
  font-weight: 600;
  cursor: pointer;
  transition: all 0.2s;
}
.back-btn-bottom:hover { background: #991b1b; }

.loading-state {
  text-align: center;
  padding: 6rem;
  color: #6b7280;
}
.spinner {
  width: 40px;
  height: 40px;
  border: 3px solid #e5e7eb;
  border-top-color: #dc2626;
  border-radius: 50%;
  animation: spin 0.8s linear infinite;
  margin: 0 auto 1rem;
}
@keyframes spin { to { transform: rotate(360deg); } }

@media (max-width: 768px) {
  .detail-header, .detail-body { padding: 1.5rem; }
  .detail-title { font-size: 1.375rem; }
}
</style>
