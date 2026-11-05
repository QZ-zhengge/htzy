<template>
  <div class="resume-page">
    <div class="container">
      <h1 class="page-title">简历管理</h1>

      <!-- 上传卡片 -->
      <div class="card upload-card">
        <div class="card-header">
          <h2 class="card-title">上传简历</h2>
        </div>
        <div class="card-body">
          <div
            class="upload-area"
            :class="{ 'drag-over': isDragOver }"
            @dragover.prevent="isDragOver = true"
            @dragleave="isDragOver = false"
            @drop.prevent="handleDrop"
            @click="triggerFileInput"
          >
            <input
              type="file"
              ref="fileInput"
              class="file-input"
              accept=".pdf,.doc,.docx"
              @change="handleFileSelect"
            />
            <div class="upload-content">
              <svg class="upload-icon" viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
                <path d="M7 16a4 4 0 01-.88-7.903A5 5 0 1115.9 6L16 6a5 5 0 011 9.9M15 13l-3-3m0 0l-3 3m3-3v12" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"/>
              </svg>
              <p class="upload-text">拖拽文件到此处，或 <span class="upload-link">点击选择文件</span></p>
              <p class="upload-hint">支持 PDF、DOC、DOCX 格式，最大 10MB</p>
            </div>
          </div>

          <div v-if="selectedFile" class="selected-file">
            <span class="file-name">{{ selectedFile.name }} ({{ formatSize(selectedFile.size) }})</span>
            <button type="button" class="btn btn-sm btn-danger" @click="clearFile">移除</button>
          </div>

          <button
            class="btn btn-primary"
            :disabled="!selectedFile || uploading"
            @click="handleUpload"
          >
            {{ uploading ? '上传中...' : '上传简历' }}
          </button>
        </div>
      </div>

      <!-- 简历列表 -->
      <div class="card">
        <div class="card-header">
          <h2 class="card-title">我的简历</h2>
        </div>
        <div class="card-body">
          <div v-if="resumes.length === 0" class="empty-state">
            <p>暂无简历，上传一份吧</p>
          </div>
          <div v-else class="table-container">
            <table class="table">
              <thead>
                <tr>
                  <th>ID</th>
                  <th>文件名</th>
                  <th>文件大小</th>
                  <th>上传时间</th>
                  <th>操作</th>
                </tr>
              </thead>
              <tbody>
                <tr v-for="resume in resumes" :key="resume.id">
                  <td>{{ resume.id }}</td>
                  <td>{{ resume.file_path ? resume.file_path.split('/').pop() : '未知文件' }}</td>
                  <td>{{ formatSize(resume.file_size) }}</td>
                  <td>{{ formatDate(resume.upload_time) }}</td>
                  <td class="action-cell">
                    <a :href="getDownloadUrl(resume.id)" class="btn btn-sm btn-outline action-btn" target="_blank" download>下载</a>
                    <button class="btn btn-sm btn-danger action-btn" @click="handleDelete(resume.id)">删除</button>
                  </td>
                </tr>
              </tbody>
            </table>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import { resumeAPI } from '../api'

const resumes = ref([])
const selectedFile = ref(null)
const uploading = ref(false)
const isDragOver = ref(false)
const fileInput = ref(null)

onMounted(async () => {
  await loadResumes()
})

async function loadResumes() {
  try {
    resumes.value = await resumeAPI.getList()
  } catch (err) {
    console.error('加载简历列表失败:', err)
  }
}

function handleFileSelect(event) {
  const file = event.target.files[0]
  if (file) {
    selectedFile.value = file
  }
}

function triggerFileInput() {
  if (fileInput.value) {
    fileInput.value.click()
  }
}

function handleDrop(event) {
  isDragOver.value = false
  const file = event.dataTransfer.files[0]
  if (file && (file.type === 'application/pdf' ||
      file.type === 'application/msword' ||
      file.type === 'application/vnd.openxmlformats-officedocument.wordprocessingml.document')) {
    selectedFile.value = file
  }
}

function clearFile() {
  selectedFile.value = null
  if (fileInput.value) {
    fileInput.value.value = ''
  }
}

async function handleUpload() {
  if (!selectedFile.value) return
  uploading.value = true
  try {
    await resumeAPI.upload(selectedFile.value)
    selectedFile.value = null
    if (fileInput.value) {
      fileInput.value.value = ''
    }
    await loadResumes()
    alert('上传成功')
  } catch (err) {
    alert('上传失败: ' + err.message)
  } finally {
    uploading.value = false
  }
}

function getDownloadUrl(id) {
  const token = sessionStorage.getItem('token') || ''
  return `/api/resumes/${id}/preview?token=${token}`
}

async function handleDelete(id) {
  if (!confirm('确定要删除这份简历吗？')) return
  try {
    await resumeAPI.delete(id)
    await loadResumes()
    alert('删除成功')
  } catch (err) {
    alert('删除失败: ' + err.message)
  }
}

function formatDate(dateStr) {
  if (!dateStr) return '-'
  const date = new Date(dateStr)
  return date.toLocaleString('zh-CN')
}

function formatSize(bytes) {
  if (!bytes || bytes === 0) return '-'
  if (bytes < 1024) return bytes + ' B'
  if (bytes < 1024 * 1024) return (bytes / 1024).toFixed(1) + ' KB'
  return (bytes / (1024 * 1024)).toFixed(1) + ' MB'
}
</script>

<style scoped>
.resume-page { padding: 2rem 0; }
.page-title { font-size: 1.75rem; font-weight: 700; color: var(--text-primary); margin-bottom: 2rem; }
.upload-card { margin-bottom: 1.5rem; }
.upload-area {
  border: 2px dashed var(--border-color);
  border-radius: var(--radius-lg);
  padding: 3rem 2rem;
  text-align: center;
  cursor: pointer;
  transition: var(--transition);
}
.upload-area:hover, .upload-area.drag-over {
  border-color: var(--primary);
  background-color: rgba(26, 86, 219, 0.02);
}
.file-input { display: none; }
.upload-icon { width: 48px; height: 48px; color: var(--text-secondary); margin-bottom: 1rem; }
.upload-text { font-size: 1rem; color: var(--text-primary); margin-bottom: 0.5rem; }
.upload-link { color: var(--primary); font-weight: 500; }
.upload-hint { font-size: 0.875rem; color: var(--text-secondary); }
.selected-file {
  display: flex; align-items: center; justify-content: space-between;
  padding: 1rem; background-color: var(--background);
  border-radius: var(--radius-md); margin: 1rem 0;
}
.file-name { font-weight: 500; color: var(--text-primary); }
.empty-state { text-align: center; padding: 3rem; color: var(--text-secondary); }
.action-cell { white-space: nowrap; }
.action-btn { display: inline-flex; align-items: center; justify-content: center; min-width: 52px; height: 28px; font-size: 0.8125rem; margin-right: 8px; }
.action-btn:last-child { margin-right: 0; }
</style>
