<template>
  <div class="interview-page">
    <div class="container">
      <!-- 面试进行中 -->
      <div v-if="interview && interview.status === 'in_progress'" class="interview-active">
        <div class="interview-layout">
          <!-- 左侧视频区域 -->
          <div class="video-section">
            <div class="video-container">
              <video 
                ref="videoRef" 
                class="video-preview" 
                autoplay 
                muted
              ></video>
              <div class="video-overlay">
                <div class="video-status">
                  <span class="status-dot" :class="{ active: isRecording }"></span>
                  {{ isRecording ? '录制中' : '准备中' }}
                </div>
              </div>
            </div>
            
            <!-- 语音识别状态 -->
            <div class="voice-status" v-if="isListening">
              <div class="voice-waves">
                <span class="wave"></span>
                <span class="wave"></span>
                <span class="wave"></span>
                <span class="wave"></span>
                <span class="wave"></span>
              </div>
              <span class="voice-text">正在听你说话...</span>
            </div>
            
            <!-- 已识别的文本 -->
            <div v-if="recognizedText" class="recognized-text">
              <p class="text-label">你的回答：</p>
              <p class="text-content">{{ recognizedText }}</p>
            </div>
          </div>

          <!-- 右侧问题区域 -->
          <div class="question-section">
            <div class="question-card">
              <div class="question-header">
                <div class="question-number">第 {{ currentIndex + 1 }} / {{ questions.length }} 题</div>
                <div class="question-timer">
                  <svg viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
                    <path d="M12 8v4l3 3m6-3a9 9 0 11-18 0 9 9 0 0118 0z" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"/>
                  </svg>
                  <span>{{ formatTime(timeRemaining) }}</span>
                </div>
              </div>
              
              <div class="question-body">
                <p class="question-text">{{ currentQuestion?.text }}</p>
                <div class="question-meta">
                  <span class="badge badge-primary">{{ currentQuestion?.type }}</span>
                  <span class="difficulty">
                    难度：
                    <span v-for="i in 5" :key="i" class="star" :class="{ active: i <= (currentQuestion?.difficulty || 0) }">★</span>
                  </span>
                </div>
              </div>

              <div class="question-actions">
                <button class="btn btn-ghost" @click="skipQuestion" :disabled="saving">
                  跳过此题
                </button>
                <button class="btn btn-primary" @click="nextQuestion" :disabled="saving">
                  {{ currentIndex === questions.length - 1 ? '结束面试' : '下一题' }}
                </button>
              </div>
            </div>

            <!-- 进度点网格 -->
            <div class="progress-dots">
              <div 
                v-for="(q, i) in questions" 
                :key="i" 
                class="dot" 
                :class="{
                  completed: i < currentIndex,
                  current: i === currentIndex,
                  pending: i > currentIndex
                }"
              ></div>
            </div>
          </div>
        </div>
      </div>

      <!-- 面试结束加载中 -->
      <div v-else-if="endingInterview" class="loading-state">
        <div class="loading-spinner"></div>
        <p>正在结束面试，请稍候...</p>
      </div>

      <!-- 面试未开始 -->
      <div v-else class="interview-start">
        <div class="card">
          <div class="card-header">
            <h2 class="card-title">面试准备</h2>
          </div>
          <div class="card-body">
            <div class="start-info">
              <p>岗位：{{ interview?.job_position || '加载中...' }}</p>
              <p>题目数量：{{ questions.length }} 题</p>
              <p>每题限时：60 秒</p>
            </div>
            <div class="start-actions">
              <button class="btn btn-primary btn-lg" @click="startInterview" :disabled="starting">
                {{ starting ? '准备中...' : '开始面试' }}
              </button>
              <router-link to="/dashboard" class="btn btn-ghost">返回</router-link>
            </div>
          </div>
        </div>
      </div>
    </div>

    <!-- Loading Modal -->
    <div v-if="showLoading" class="loading-overlay">
      <div class="loading-modal">
        <div class="loading-spinner-large"></div>
        <h3 class="loading-title">AI 正在为你生成评估报告...</h3>
        <p class="loading-subtitle">正在综合分析你的面试表现，请稍候</p>
        <div class="loading-tip">
          <span class="tip-icon">💡</span>
          <span class="tip-text">{{ loadingTip }}</span>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted, onUnmounted, nextTick } from 'vue'
import { useRoute, useRouter } from 'vue-router'
import { interviewAPI } from '../api'

const route = useRoute()
const router = useRouter()
const interviewId = route.params.id

const interview = ref(null)
const questions = ref([])
const currentIndex = ref(0)
const currentQuestion = ref(null)
const timeRemaining = ref(60)
const isRecording = ref(false)
const isListening = ref(false)
const recognizedText = ref('')
const saving = ref(false)
const endingInterview = ref(false)
const starting = ref(false)

// 视频相关
const videoRef = ref(null)
let videoStream = null
let frameCaptureInterval = null
let timerInterval = null

// 语音识别相关
let recognition = null
let isEndingInterview = false
const showLoading = ref(false)
const loadingTip = ref('')

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
let isSavingAnswer = false

onMounted(async () => {
  await loadInterview()
})

onUnmounted(() => {
  cleanup()
})

async function loadInterview() {
  try {
    interview.value = await interviewAPI.getDetail(interviewId)
    questions.value = interview.value.questions || []
    
    if (questions.value.length > 0) {
      currentQuestion.value = questions.value[0]
    }
  } catch (err) {
    console.error('加载面试信息失败:', err)
    alert('加载面试信息失败')
    router.push('/dashboard')
  }
}

async function startInterview() {
  starting.value = true
  
  try {
    await interviewAPI.start(interviewId)
    interview.value.status = 'in_progress'
    
    // 初始化摄像头
    await initCamera()
    
    // 初始化语音识别
    initSpeechRecognition()
    
    // 开始计时
    startTimer()
    
    // 开始抓帧
    startFrameCapture()
  } catch (err) {
    console.error('开始面试失败:', err)
    alert('开始面试失败: ' + err.message)
  } finally {
    starting.value = false
  }
}

async function initCamera() {
  try {
    videoStream = await navigator.mediaDevices.getUserMedia({
      video: { width: 640, height: 480 },
      audio: false
    })
    
    await nextTick()
    if (videoRef.value) {
      videoRef.value.srcObject = videoStream
    }
    isRecording.value = true
  } catch (err) {
    console.error('摄像头初始化失败:', err)
    alert('无法访问摄像头，请检查权限设置')
  }
}

function initSpeechRecognition() {
  const SpeechRecognition = window.SpeechRecognition || window.webkitSpeechRecognition
  
  if (!SpeechRecognition) {
    console.warn('浏览器不支持语音识别')
    return
  }
  
  recognition = new SpeechRecognition()
  recognition.continuous = true
  recognition.interimResults = true
  recognition.lang = 'zh-CN'
  
  recognition.onresult = (event) => {
    let transcript = ''
    for (let i = event.resultIndex; i < event.results.length; i++) {
      transcript += event.results[i][0].transcript
    }
    recognizedText.value = transcript
  }
  
  recognition.onstart = () => {
    isListening.value = true
  }
  
  recognition.onend = () => {
    isListening.value = false
    // Auto-restart if still in interview (Speech API stops after silence)
    if (!endingInterview.value && recognition) {
      setTimeout(() => {
        try { recognition.start() } catch(e) {}
      }, 300)
    }
  }

  recognition.onerror = (event) => {
    console.error('语音识别错误:', event.error)
    isListening.value = false
    // Auto-restart on transient errors
    if (!endingInterview.value && recognition && event.error !== 'not-allowed') {
      setTimeout(() => {
        try { recognition.start() } catch(e) {}
      }, 500)
    }
  }
  
  // 启动语音识别
  recognition.start()
}

function startTimer() {
  timeRemaining.value = 60
  
  timerInterval = setInterval(() => {
    if (timeRemaining.value > 0) {
      timeRemaining.value--
    } else {
      // 时间到，自动跳到下一题
      nextQuestion()
    }
  }, 1000)
}

function startFrameCapture() {
  // 每5秒抓取一帧进行分析
  frameCaptureInterval = setInterval(() => {
    captureFrame()
  }, 5000)
}

async function captureFrame() {
  if (!videoRef.value || !isRecording.value) return
  
  try {
    const canvas = document.createElement('canvas')
    canvas.width = videoRef.value.videoWidth
    canvas.height = videoRef.value.videoHeight
    const ctx = canvas.getContext('2d')
    ctx.drawImage(videoRef.value, 0, 0)
    
    const imageData = canvas.toDataURL('image/jpeg', 0.8)
    
    await interviewAPI.analyzeFace(interviewId, {
      frame_number: currentIndex.value + 1,
      image: imageData
    })
  } catch (err) {
    console.error('抓帧分析失败:', err)
  }
}

async function saveAnswer() {
  if (isSavingAnswer) return
  isSavingAnswer = true
  
  try {
    await interviewAPI.submitAnswer(interviewId, {
      question_id: questions.value[currentIndex.value]?.id || currentIndex.value + 1,
      answer_text: recognizedText.value || ''
    })
    
    // 如果有语音识别结果，发送到后端分析
    if (recognizedText.value) {
      await interviewAPI.analyzeVoice(interviewId, {
        text: recognizedText.value,
        questionIndex: currentIndex.value
      })
    }
  } catch (err) {
    console.error('保存答案失败:', err)
  } finally {
    isSavingAnswer = false
  }
}

async function nextQuestion() {
  if (saving.value) return
  saving.value = true
  
  try {
    // 保存当前答案
    await saveAnswer()
    
    // 重置状态
    recognizedText.value = ''
    timeRemaining.value = 60
    
    if (currentIndex.value < questions.value.length - 1) {
      // 下一题
      currentIndex.value++
      currentQuestion.value = questions.value[currentIndex.value]
    } else {
      // 面试结束
      await endInterview()
    }
  } finally {
    saving.value = false
  }
}

async function skipQuestion() {
  if (saving.value) return
  saving.value = true
  
  try {
    recognizedText.value = ''
    timeRemaining.value = 60
    
    if (currentIndex.value < questions.value.length - 1) {
      currentIndex.value++
      currentQuestion.value = questions.value[currentIndex.value]
    } else {
      await endInterview()
    }
  } finally {
    saving.value = false
  }
}

async function endInterview() {
  if (isEndingInterview) return
  isEndingInterview = true

  try {
    endingInterview.value = true
    cleanup()

    // Show loading dialog with tips
    showLoading.value = true
    loadingTip.value = getRandomTip()

    await interviewAPI.end(interviewId)

    showLoading.value = false
    router.push(`/results/${interviewId}`)
  } catch (err) {
    showLoading.value = false
    console.error('结束面试失败:', err)
    alert('结束面试失败: ' + err.message)
    router.push('/dashboard')
  }
}

function cleanup() {
  // 停止视频流
  if (videoStream) {
    videoStream.getTracks().forEach(track => track.stop())
    videoStream = null
  }
  
  // 清除定时器
  if (frameCaptureInterval) {
    clearInterval(frameCaptureInterval)
    frameCaptureInterval = null
  }
  
  if (timerInterval) {
    clearInterval(timerInterval)
    timerInterval = null
  }
  
  // 停止语音识别
  if (recognition) {
    try {
      recognition.stop()
    } catch (e) {}
    recognition = null
  }
  
  isRecording.value = false
  isListening.value = false
}

function formatTime(seconds) {
  const mins = Math.floor(seconds / 60)
  const secs = seconds % 60
  return `${mins.toString().padStart(2, '0')}:${secs.toString().padStart(2, '0')}`
}
</script>

<style scoped>
.interview-page {
  padding: 1.5rem 0;
  min-height: calc(100vh - 64px);
  background-color: var(--background);
}

.interview-active {
  height: calc(100vh - 96px);
}

.interview-layout {
  display: grid;
  grid-template-columns: 2fr 1fr;
  gap: 1.5rem;
  height: 100%;
}

/* 视频区域 */
.video-section {
  display: flex;
  flex-direction: column;
  gap: 1rem;
}

.video-container {
  position: relative;
  background: #000;
  border-radius: 1rem;
  overflow: hidden;
  aspect-ratio: 4/3;
}

.video-preview {
  width: 100%;
  height: 100%;
  object-fit: cover;
}

.video-overlay {
  position: absolute;
  top: 1rem;
  left: 1rem;
}

.video-status {
  display: flex;
  align-items: center;
  gap: 0.5rem;
  padding: 0.5rem 1rem;
  background: rgba(0, 0, 0, 0.6);
  color: white;
  border-radius: 9999px;
  font-size: 0.875rem;
}

.status-dot {
  width: 8px;
  height: 8px;
  background: var(--text-secondary);
  border-radius: 50%;
}

.status-dot.active {
  background: var(--danger);
  animation: pulse 1s infinite;
}

.voice-status {
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 1rem;
  padding: 1rem;
  background: white;
  border-radius: 1rem;
}

.voice-waves {
  display: flex;
  align-items: center;
  gap: 4px;
}

.wave {
  width: 4px;
  height: 20px;
  background: var(--primary);
  border-radius: 2px;
  animation: wave 0.5s ease-in-out infinite;
}

.wave:nth-child(2) { animation-delay: 0.1s; }
.wave:nth-child(3) { animation-delay: 0.2s; }
.wave:nth-child(4) { animation-delay: 0.3s; }
.wave:nth-child(5) { animation-delay: 0.4s; }

@keyframes wave {
  0%, 100% { height: 10px; }
  50% { height: 25px; }
}

.voice-text {
  font-size: 0.9375rem;
  color: var(--text-primary);
}

.recognized-text {
  padding: 1rem;
  background: white;
  border-radius: 1rem;
}

.text-label {
  font-size: 0.875rem;
  color: var(--text-secondary);
  margin-bottom: 0.5rem;
}

.text-content {
  font-size: 1rem;
  color: var(--text-primary);
  line-height: 1.6;
}

/* 问题区域 */
.question-section {
  display: flex;
  flex-direction: column;
  gap: 1.5rem;
}

.question-card {
  background: white;
  border-radius: 1rem;
  box-shadow: var(--shadow-lg);
  flex: 1;
  display: flex;
  flex-direction: column;
}

.question-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 1.5rem;
  border-bottom: 1px solid var(--border-color);
}

.question-number {
  font-weight: 600;
  color: var(--text-primary);
}

.question-timer {
  display: flex;
  align-items: center;
  gap: 0.5rem;
  font-size: 1.25rem;
  font-weight: 600;
  color: var(--danger);
}

.question-timer svg {
  width: 24px;
  height: 24px;
}

.question-body {
  flex: 1;
  padding: 1.5rem;
}

.question-text {
  font-size: 1.125rem;
  line-height: 1.8;
  color: var(--text-primary);
  margin-bottom: 1rem;
}

.question-meta {
  display: flex;
  align-items: center;
  gap: 1rem;
}

.difficulty {
  font-size: 0.875rem;
  color: var(--text-secondary);
}

.star {
  color: #d1d5db;
}

.star.active {
  color: #f59e0b;
}

.question-actions {
  display: flex;
  justify-content: space-between;
  padding: 1.5rem;
  border-top: 1px solid var(--border-color);
}

/* 进度点 */
.progress-dots {
  display: flex;
  flex-wrap: wrap;
  gap: 0.5rem;
  padding: 1rem;
  background: white;
  border-radius: 1rem;
}

.dot {
  width: 12px;
  height: 12px;
  border-radius: 50%;
  background: #e5e7eb;
}

.dot.completed {
  background: var(--success);
}

.dot.current {
  background: var(--primary);
  transform: scale(1.2);
}

.dot.pending {
  background: #e5e7eb;
}

/* 加载状态 */
.loading-state {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  height: calc(100vh - 96px);
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

/* 面试开始 */
.interview-start {
  display: flex;
  align-items: center;
  justify-content: center;
  min-height: calc(100vh - 96px);
}

.interview-start .card {
  max-width: 500px;
  width: 100%;
}

.start-info {
  margin-bottom: 2rem;
}

.start-info p {
  padding: 0.5rem 0;
  font-size: 1rem;
  color: var(--text-primary);
}

.start-actions {
  display: flex;
  gap: 1rem;
  justify-content: center;
}

/* 响应式 */
@media (max-width: 1024px) {
  .interview-layout {
    grid-template-columns: 1fr;
    height: auto;
  }

  .video-container {
    max-height: 300px;
  }
}

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