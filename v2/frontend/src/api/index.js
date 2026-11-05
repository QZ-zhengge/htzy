import axios from 'axios'

const api = axios.create({
  baseURL: '/api',
  timeout: 60000,
  headers: { 'Content-Type': 'application/json' }
})

// Request interceptor - auto add auth token
api.interceptors.request.use((config) => {
  const token = sessionStorage.getItem('token')
  if (token) config.headers.Authorization = `Bearer ${token}`
  return config
})

// Response interceptor - handle errors
api.interceptors.response.use(
  (response) => response.data,
  (error) => {
    if (error.response) {
      const { status, data } = error.response
      if (status === 401) {
        // Only redirect on 401 if user was previously logged in (token expired)
        const hasToken = !!sessionStorage.getItem('token')
        if (hasToken) {
          sessionStorage.removeItem('token')
          sessionStorage.removeItem('user')
          sessionStorage.removeItem('loginTime')
          window.location.href = '/login'
        }
      }
      return Promise.reject(new Error(data.error || data.message || 'Request failed'))
    }
    return Promise.reject(error)
  }
)

// Auth
export const authAPI = {
  register: (data) => api.post('/auth/register', data),
  login: (data) => api.post('/auth/login', data),
  sendCode: (email) => api.post('/auth/send-code', { email }),
  verifyCode: (email, code) => api.post('/auth/verify-code', { email, code }),
}

// User
export const userAPI = {
  getProfile: () => api.get('/user/profile'),
  updateProfile: (data) => api.put('/user/profile', data)
}

// Resume - use JSON+base64 to match backend
export const resumeAPI = {
  getList: () => api.get('/resumes'),
  upload: async (file) => {
    const reader = new FileReader()
    const content = await new Promise((resolve) => {
      reader.onload = () => resolve(reader.result.split(',')[1] || '')
      reader.readAsDataURL(file)
    })
    return api.post('/resumes', {
      filename: file.name,
      content_base64: content
    })
  },
  getDetail: (id) => api.get(`/resumes/${id}`),
  delete: (id) => api.delete(`/resumes/${id}`)
}

// Jobs
export const jobAPI = {
  getList: (params) => api.get('/jobs', { params }),
  getDetail: (id) => api.get(`/jobs/${id}`)
}

// Red Stories
export const redStoryAPI = {
  getList: () => api.get('/redstories'),
  getDetail: (id) => api.get(`/redstories/${id}`)
}

// Admin
export const adminAPI = {
  addAdmin: (data) => api.post('/admin/super/add-admin', data),
  getStudentRoster: (params) => api.get('/admin/students', { params }),
  uploadRoster: (data) => api.post('/admin/students/upload', data),
  uploadRosterExcel: (formData) => api.post('/admin/students/upload-excel', formData, { headers: { 'Content-Type': 'multipart/form-data' } }),
  getRegisteredStudents: () => api.get('/admin/students/registered'),
  getScores: (params) => api.get('/admin/scores', { params }),
  getStats: () => api.get('/admin/stats'),
}

// Delivery
export const deliveryAPI = {
  toggle: (data) => api.post('/delivery/toggle', data),
  getConfig: () => api.get('/delivery/config'),
  getLogs: () => api.get('/delivery/logs'),
  getSuggested: () => api.get('/delivery/suggested'),
}

// Interview
export const interviewAPI = {
  create: (data) => api.post('/interviews', data),
  getList: () => api.get('/interviews'),
  getDetail: (id) => api.get(`/interviews/${id}`),
  start: (id) => api.post(`/interviews/${id}/start`),
  end: (id) => api.post(`/interviews/${id}/end`),
  submitAnswer: (id, data) => api.post(`/interviews/${id}/answers`, data),
  analyzeFace: (id, data) => api.post(`/interviews/${id}/analyze_face`, data),
  analyzeVoice: (id, data) => api.post(`/interviews/${id}/analyze_voice`, data),
  getResults: (id) => api.get(`/interviews/${id}/results`)
}

export default api
