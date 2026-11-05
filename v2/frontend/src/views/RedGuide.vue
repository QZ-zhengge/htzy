<template>
  <div class="red-guide-page">
    <!-- Hero Banner -->
    <div class="red-hero">
      <div class="red-hero-bg"></div>
      <div class="container red-hero-content">
        <h1 class="red-hero-title">红色指引</h1>
        <p class="red-hero-sub">传承红色基因 · 铸就科技报国之魂</p>
      </div>
    </div>

    <div class="container">
      <!-- Category Tabs -->
      <div class="category-tabs">
        <button
          v-for="cat in categories"
          :key="cat.value"
          class="tab-btn"
          :class="{ active: activeCategory === cat.value }"
          @click="activeCategory = cat.value"
        >
          {{ cat.label }}
        </button>
      </div>

      <!-- Stories List -->
      <div v-if="loading" class="loading-state">
        <div class="spinner"></div>
        <p>加载中...</p>
      </div>

      <div v-else-if="filteredStories.length === 0" class="empty-state">
        <p>暂无相关内容</p>
      </div>

      <div v-else class="stories-grid">
        <div
          v-for="story in filteredStories"
          :key="story.id"
          class="story-card"
          @click="$router.push(`/red-guide/${story.id}`)"
        >
          <div class="story-category-badge">{{ getCategoryLabel(story.category) }}</div>
          <h3 class="story-title">{{ story.title }}</h3>
          <p class="story-summary">{{ story.summary }}</p>
          <div class="story-footer">
            <span class="story-source">{{ story.source_name }}</span>
            <span class="story-views">阅读 {{ story.view_count }}</span>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, computed, onMounted } from 'vue'
import { redStoryAPI } from '../api'

const stories = ref([])
const loading = ref(true)
const activeCategory = ref('all')

const categories = [
  { label: '全部', value: 'all' },
  { label: '计算机先驱', value: 'computer_pioneer' },
  { label: '芯片之光', value: 'chip_pioneer' },
  { label: '两弹一星', value: 'two_bombs_one_satellite' }
]

const categoryMap = {
  computer_pioneer: '计算机先驱',
  chip_pioneer: '芯片之光',
  two_bombs_one_satellite: '两弹一星'
}

const filteredStories = computed(() => {
  if (activeCategory.value === 'all') return stories.value
  return stories.value.filter(s => s.category === activeCategory.value)
})

function getCategoryLabel(cat) {
  return categoryMap[cat] || cat
}

onMounted(async () => {
  try {
    stories.value = await redStoryAPI.getList()
  } catch (err) {
    console.error('Load stories failed:', err)
  } finally {
    loading.value = false
  }
})
</script>

<style scoped>
.red-guide-page { min-height: 100vh; background: #f8fafc; }

.red-hero {
  position: relative;
  padding: 4rem 0 3rem;
  overflow: hidden;
  text-align: center;
}
.red-hero-bg {
  position: absolute;
  inset: 0;
  background: linear-gradient(135deg, #991b1b 0%, #b91c1c 30%, #dc2626 60%, #991b1b 100%);
}
.red-hero-bg::before {
  content: '';
  position: absolute;
  inset: 0;
  background: url("data:image/svg+xml,%3Csvg width='60' height='60' viewBox='0 0 60 60' xmlns='http://www.w3.org/2000/svg'%3E%3Cg fill='none' fill-rule='evenodd'%3E%3Cg fill='%23ffffff' fill-opacity='0.05'%3E%3Cpath d='M36 34v-4h-2v4h-4v2h4v4h2v-4h4v-2h-4zm0-30V0h-2v4h-4v2h4v4h2V6h4V4h-4zM6 34v-4H4v4H0v2h4v4h2v-4h4v-2H6zM6 4V0H4v4H0v2h4v4h2V6h4V4H6z'/%3E%3C/g%3E%3C/g%3E%3C/svg%3E");
}
.red-hero-content { position: relative; z-index: 1; }
.red-hero-title {
  font-size: 2.5rem;
  font-weight: 800;
  color: white;
  margin-bottom: 0.75rem;
  text-shadow: 0 2px 8px rgba(0,0,0,0.2);
}
.red-hero-sub {
  font-size: 1.125rem;
  color: rgba(255,255,255,0.85);
}

.category-tabs {
  display: flex;
  gap: 0.75rem;
  padding: 1.5rem 0;
  flex-wrap: wrap;
}
.tab-btn {
  padding: 0.5rem 1.25rem;
  border-radius: 9999px;
  border: 1px solid #d1d5db;
  background: white;
  color: #374151;
  font-size: 0.9375rem;
  cursor: pointer;
  transition: all 0.2s;
}
.tab-btn:hover { border-color: #dc2626; color: #dc2626; }
.tab-btn.active {
  background: #dc2626;
  color: white;
  border-color: #dc2626;
}

.stories-grid {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(340px, 1fr));
  gap: 1.5rem;
  padding-bottom: 3rem;
}

.story-card {
  background: white;
  border-radius: 12px;
  padding: 1.75rem;
  box-shadow: 0 1px 3px rgba(0,0,0,0.08);
  border: 1px solid #f1f5f9;
  cursor: pointer;
  transition: all 0.3s;
  position: relative;
  overflow: hidden;
}
.story-card::before {
  content: '';
  position: absolute;
  top: 0;
  left: 0;
  width: 4px;
  height: 100%;
  background: linear-gradient(180deg, #dc2626, #991b1b);
}
.story-card:hover {
  transform: translateY(-4px);
  box-shadow: 0 12px 24px rgba(0,0,0,0.1);
  border-color: #fecaca;
}

.story-category-badge {
  display: inline-block;
  padding: 0.25rem 0.75rem;
  border-radius: 9999px;
  font-size: 0.75rem;
  font-weight: 600;
  background: #fef2f2;
  color: #dc2626;
  margin-bottom: 0.75rem;
}

.story-title {
  font-size: 1.125rem;
  font-weight: 700;
  color: #1f2937;
  margin-bottom: 0.625rem;
  line-height: 1.4;
}

.story-summary {
  font-size: 0.875rem;
  color: #6b7280;
  line-height: 1.6;
  margin-bottom: 1rem;
  display: -webkit-box;
  -webkit-line-clamp: 3;
  -webkit-box-orient: vertical;
  overflow: hidden;
}

.story-footer {
  display: flex;
  justify-content: space-between;
  align-items: center;
  font-size: 0.8125rem;
  color: #9ca3af;
  padding-top: 0.75rem;
  border-top: 1px solid #f3f4f6;
}

.loading-state {
  text-align: center;
  padding: 4rem;
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

.empty-state {
  text-align: center;
  padding: 4rem;
  color: #9ca3af;
}

@media (max-width: 768px) {
  .stories-grid { grid-template-columns: 1fr; }
  .red-hero-title { font-size: 1.75rem; }
}
</style>
