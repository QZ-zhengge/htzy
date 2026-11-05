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
        <h2>学生管理</h2>
      </div>

      <!-- Tabs -->
      <div class="tabs">
        <button :class="{ active: tab === 'roster' }" @click="tab = 'roster'">花名册 ({{ rosterTotal }})</button>
        <button :class="{ active: tab === 'registered' }" @click="tab = 'registered'; loadRegistered()">已注册 ({{ registeredList.length }})</button>
        <button :class="{ active: tab === 'upload' }" @click="tab = 'upload'">上传花名册</button>
      </div>

      <!-- Roster -->
      <div v-if="tab === 'roster'" class="tab-content">
        <div class="search-bar">
          <input type="text" v-model="searchKey" placeholder="搜索学号或姓名..." @input="loadRoster" />
        </div>
        <table class="data-table">
          <thead><tr><th>学号</th><th>姓名</th><th>班级</th><th>院系</th></tr></thead>
          <tbody>
            <tr v-for="s in rosterList" :key="s.id">
              <td>{{ s.student_id }}</td><td>{{ s.real_name }}</td><td>{{ s.class_name }}</td><td>{{ s.department }}</td>
            </tr>
          </tbody>
        </table>
        <p v-if="rosterList.length === 0" class="empty">暂无数据</p>
      </div>

      <!-- Registered -->
      <div v-if="tab === 'registered'" class="tab-content">
        <table class="data-table">
          <thead><tr><th>学号</th><th>姓名</th><th>班级</th><th>用户名</th><th>邮箱</th><th>已验证</th><th>面试数</th><th>注册时间</th></tr></thead>
          <tbody>
            <tr v-for="s in registeredList" :key="s.id">
              <td>{{ s.student_id }}</td><td>{{ s.real_name }}</td><td>{{ s.class_name }}</td>
              <td>{{ s.username }}</td><td>{{ s.email }}</td>
              <td><span :class="s.is_verified ? 'badge-green' : 'badge-gray'">{{ s.is_verified ? '是' : '否' }}</span></td>
              <td>{{ s.interview_count }}</td>
              <td>{{ formatDate(s.created_at) }}</td>
            </tr>
          </tbody>
        </table>
        <p v-if="registeredList.length === 0" class="empty">暂无注册学生</p>
      </div>

      <!-- Upload -->
      <div v-if="tab === 'upload'" class="tab-content">
        <div class="upload-area">
          <p>上传学生花名册（Excel .xlsx 格式）</p>
          <p class="hint">表头要求：学号、姓名、班级、院系（后两列可选）</p>
          <div class="upload-file-row">
            <input type="file" ref="fileInput" accept=".xlsx,.xls" @change="handleFileSelect" style="display:none" />
            <button class="btn btn-outline" @click="$refs.fileInput.click()">选择文件</button>
            <span v-if="uploadFile" class="file-name">{{ uploadFile.name }}</span>
          </div>
          <button class="btn btn-primary" @click="handleExcelUpload" :disabled="uploading || !uploadFile" style="margin-top:1rem">
            {{ uploading ? '上传中...' : '上传花名册' }}
          </button>
          <p v-if="uploadResult" class="upload-result">{{ uploadResult }}</p>

          <hr style="margin: 1.5rem 0; border-color: #e5e7eb" />
          <p class="hint">或者粘贴 JSON 数据（兼容旧版）：</p>
          <textarea v-model="uploadJson" rows="6" placeholder='[{"student_id":"2024001","real_name":"张三","class_name":"CS2401"}]'></textarea>
          <button class="btn btn-outline" @click="handleJsonUpload" :disabled="uploading">
            {{ uploading ? '上传中...' : 'JSON 上传' }}
          </button>
          <p v-if="jsonResult" class="upload-result">{{ jsonResult }}</p>
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
const tab = ref('roster')
const rosterList = ref([])
const rosterTotal = ref(0)
const searchKey = ref('')
const registeredList = ref([])
const uploadJson = ref('')
const uploadFile = ref(null)
const uploading = ref(false)
const uploadResult = ref('')
const jsonResult = ref('')

onMounted(() => loadRoster())

async function loadRoster() {
  try {
    const r = await adminAPI.getStudentRoster({ keyword: searchKey.value, page: 1, size: 200 })
    rosterList.value = r.data || []
    rosterTotal.value = r.total || 0
  } catch {}
}

async function loadRegistered() {
  try { registeredList.value = await adminAPI.getRegisteredStudents() } catch {}
}

async function handleJsonUpload() {
  uploading.value = true; jsonResult.value = ''
  try {
    const data = JSON.parse(uploadJson.value)
    const r = await adminAPI.uploadRoster(data)
    jsonResult.value = `导入 ${r.imported} 条，更新 ${r.updated} 条`
    loadRoster()
  } catch (e) { jsonResult.value = '上传失败: ' + e.message }
  finally { uploading.value = false }
}

function handleFileSelect(e) {
  uploadFile.value = e.target.files[0]
  uploadResult.value = ''
}

async function handleExcelUpload() {
  if (!uploadFile.value) return
  uploading.value = true; uploadResult.value = ''
  try {
    const formData = new FormData()
    formData.append('file', uploadFile.value)
    const r = await adminAPI.uploadRosterExcel(formData)
    uploadResult.value = `成功导入 ${r.imported} 条，更新 ${r.updated} 条，共 ${r.total} 条`
    uploadFile.value = null
    loadRoster()
  } catch (e) { uploadResult.value = '上传失败: ' + e.message }
  finally { uploading.value = false }
}

function formatDate(d) { return d ? new Date(d).toLocaleDateString('zh-CN') : '-' }
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
.tabs { display: flex; gap: 0.5rem; margin-bottom: 1.5rem; }
.tabs button { padding: 0.5rem 1.25rem; border: 1px solid #d1d5db; border-radius: 8px; background: white; cursor: pointer; font-size: 0.9375rem; }
.tabs button.active { background: #0ea5e9; color: white; border-color: #0ea5e9; }
.tab-content { background: white; border-radius: 12px; padding: 1.5rem; box-shadow: 0 1px 3px rgba(0,0,0,0.08); }
.search-bar { margin-bottom: 1rem; }
.search-bar input { padding: 0.5rem 0.75rem; border: 1px solid #d1d5db; border-radius: 8px; width: 300px; }
.data-table { width: 100%; border-collapse: collapse; }
.data-table th, .data-table td { padding: 0.75rem 1rem; text-align: left; border-bottom: 1px solid #f1f5f9; font-size: 0.875rem; }
.data-table th { color: #6b7280; font-weight: 600; background: #f8fafc; }
.badge-green { background: #dcfce7; color: #166534; padding: 0.125rem 0.5rem; border-radius: 9999px; font-size: 0.75rem; }
.badge-gray { background: #f1f5f9; color: #64748b; padding: 0.125rem 0.5rem; border-radius: 9999px; font-size: 0.75rem; }
.empty { color: #9ca3af; text-align: center; padding: 2rem; }
.upload-area textarea { width: 100%; border: 1px solid #d1d5db; border-radius: 8px; padding: 0.75rem; font-family: monospace; font-size: 0.8125rem; margin: 0.75rem 0; }
.hint { font-size: 0.8125rem; color: #6b7280; margin-top: 0.5rem; }
.upload-result { margin-top: 1rem; color: #059669; font-weight: 500; }
.upload-file-row { display: flex; align-items: center; gap: 0.75rem; margin-top: 0.75rem; }
.file-name { color: #374151; font-size: 0.875rem; }
</style>
