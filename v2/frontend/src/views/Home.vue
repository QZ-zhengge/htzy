<template>
  <div class="home">
    <!-- Hero区域：深色科技风 -->
    <section class="hero">
      <div class="hero-bg"></div>
      <div class="container">
        <div class="hero-content">
          <div class="hero-badge">
            <span class="badge-dot"></span>
            <span>Powered by DeepSeek AI</span>
          </div>
          <div class="hero-logo-wrap">
            <img src="/logo-large.png" alt="红途职引" class="hero-logo" />
          </div>
          <h1 class="hero-title"><span class="highlight">红途职引</span> 平台</h1>
          <p class="hero-subtitle">基于大语言模型的智能面试模拟平台，实时分析你的简历、回答、微表情与语音，生成全方位评估报告</p>
          <div class="hero-actions">
            <router-link to="/register" class="btn btn-primary btn-lg">
              <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><polygon points="5 3 19 12 5 21 5 3"/></svg>
              立即体验
            </router-link>
            <router-link to="/jobs" class="btn btn-ghost-light btn-lg">
              <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="11" cy="11" r="8"/><line x1="21" y1="21" x2="16.65" y2="16.65"/></svg>
              浏览岗位
            </router-link>
          </div>
          <div class="hero-stats">
            <div class="stat">
              <div class="stat-value">20+</div>
              <div class="stat-label">AI面试题</div>
            </div>
            <div class="stat-divider"></div>
            <div class="stat">
              <div class="stat-value">5</div>
              <div class="stat-label">评估维度</div>
            </div>
            <div class="stat-divider"></div>
            <div class="stat">
              <div class="stat-value">&lt;3s</div>
              <div class="stat-label">实时分析</div>
            </div>
          </div>
        </div>
        <div class="hero-visual">
          <div class="terminal-card">
            <div class="terminal-bar">
              <span class="terminal-dot red"></span>
              <span class="terminal-dot yellow"></span>
              <span class="terminal-dot green"></span>
              <span class="terminal-title">interview_analysis.py</span>
            </div>
            <div class="terminal-body">
              <div class="code-line"><span class="code-keyword">import</span> <span class="code-module">htzy</span></div>
              <div class="code-line"><span class="code-keyword">from</span> <span class="code-module">htzy.ai</span> <span class="code-keyword">import</span> DeepSeek</div>
              <div class="code-line">&nbsp;</div>
              <div class="code-line"><span class="code-comment"># Real-time interview analysis</span></div>
              <div class="code-line"><span class="code-var">model</span> = <span class="code-module">DeepSeek</span>(<span class="code-string">"deepseek-chat"</span>)</div>
              <div class="code-line"><span class="code-var">result</span> = model.<span class="code-fn">analyze</span>(</div>
              <div class="code-line">&nbsp;&nbsp;resume=<span class="code-var">resume</span>,</div>
              <div class="code-line">&nbsp;&nbsp;position=<span class="code-string">"Backend Engineer"</span>,</div>
              <div class="code-line">&nbsp;&nbsp;questions=<span class="code-var">20</span></div>
              <div class="code-line">)</div>
              <div class="code-line">&nbsp;</div>
              <div class="code-line"><span class="code-keyword">print</span>(<span class="code-string">f"Score: </span><span class="code-var">{result.overall}</span><span class="code-string">/100"</span>)</div>
            </div>
          </div>
        </div>
      </div>
    </section>

    <!-- 技术特性 -->
    <section class="features">
      <div class="container">
        <h2 class="section-title">核心技术栈</h2>
        <p class="section-subtitle">C++20 高性能后端 + Vue3 响应式前端 + DeepSeek 大模型</p>
        <div class="features-grid">
          <div class="feature-card" v-for="(feature, index) in features" :key="index">
            <div class="feature-icon-box" :style="{ background: feature.bg }">
              <div class="feature-icon" v-html="feature.svg"></div>
            </div>
            <h3 class="feature-title">{{ feature.title }}</h3>
            <p class="feature-desc">{{ feature.desc }}</p>
            <div class="feature-tag">{{ feature.tag }}</div>
          </div>
        </div>
      </div>
    </section>

    <!-- 工作流程 -->
    <section class="workflow">
      <div class="container">
        <h2 class="section-title">工作流程</h2>
        <p class="section-subtitle">四步完成 AI 模拟面试</p>
        <div class="workflow-steps">
          <div class="workflow-step" v-for="(step, index) in steps" :key="index">
            <div class="step-number">{{ index + 1 }}</div>
            <div class="step-line" v-if="index < steps.length - 1"></div>
            <h3 class="step-title">{{ step.title }}</h3>
            <p class="step-desc">{{ step.desc }}</p>
          </div>
        </div>
      </div>
    </section>

    <!-- CTA -->
    <section class="cta">
      <div class="container">
        <div class="cta-box">
          <h2 class="cta-title">开始你的 AI 面试练习</h2>
          <p class="cta-subtitle">上传简历，选择岗位，AI 自动生成针对性题目并实时评估</p>
          <router-link to="/register" class="btn btn-primary btn-lg">免费注册</router-link>
        </div>
      </div>
    </section>

    <!-- Footer -->
    <footer class="footer">
      <div class="container">
        <div class="footer-inner">
          <div class="footer-brand">
            <span class="footer-logo">HTZY</span>
            <span class="footer-desc">红途职引 · AI面试系统</span>
          </div>
          <p class="footer-copy">&copy; 2026 HTZY. Built with C++20 & Vue3</p>
        </div>
      </div>
    </footer>
  </div>
</template>

<script setup>
const features = [
  {
    title: '简历智能分析',
    desc: 'DeepSeek 大模型深度解析简历，提取技能清单、项目经验，匹配目标岗位',
    tag: 'DeepSeek API',
    bg: 'rgba(14, 165, 233, 0.08)',
    svg: '<svg width="28" height="28" viewBox="0 0 24 24" fill="none" stroke="#0ea5e9" stroke-width="2"><path d="M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8z"/><polyline points="14 2 14 8 20 8"/><line x1="16" y1="13" x2="8" y2="13"/><line x1="16" y1="17" x2="8" y2="17"/><polyline points="10 9 9 9 8 9"/></svg>'
  },
  {
    title: 'AI 自动出题',
    desc: '根据简历和岗位自动生成 20+ 面试题，涵盖技术、行为、情境等类型',
    tag: '20+ Questions',
    bg: 'rgba(6, 182, 212, 0.08)',
    svg: '<svg width="28" height="28" viewBox="0 0 24 24" fill="none" stroke="#06b6d4" stroke-width="2"><circle cx="12" cy="12" r="10"/><path d="M9.09 9a3 3 0 0 1 5.83 1c0 2-3 3-3 3"/><line x1="12" y1="17" x2="12.01" y2="17"/></svg>'
  },
  {
    title: '多模态评估',
    desc: '摄像头微表情分析 + 语音稳定性评分 + 文本语义评分，5 维度综合打分',
    tag: 'WebSocket + STT',
    bg: 'rgba(16, 185, 129, 0.08)',
    svg: '<svg width="28" height="28" viewBox="0 0 24 24" fill="none" stroke="#10b981" stroke-width="2"><path d="M23 7l-7 5 7 5V7z"/><rect x="1" y="5" width="15" height="14" rx="2" ry="2"/></svg>'
  },
  {
    title: '性能与安全',
    desc: 'C++20 高性能后端，MySQL C API 直连，JWT 鉴权，参数化 SQL 防注入',
    tag: 'C++20 / MySQL',
    bg: 'rgba(245, 158, 11, 0.08)',
    svg: '<svg width="28" height="28" viewBox="0 0 24 24" fill="none" stroke="#f59e0b" stroke-width="2"><rect x="3" y="11" width="18" height="11" rx="2" ry="2"/><path d="M7 11V7a5 5 0 0 1 10 0v4"/></svg>'
  }
]

const steps = [
  { title: '上传简历', desc: '支持 TXT/PDF 格式，AI 自动提取关键信息' },
  { title: '选择岗位', desc: '从爬虫库中浏览岗位，或自定义目标岗位' },
  { title: '模拟面试', desc: '逐题作答，摄像头+语音实时分析' },
  { title: '查看报告', desc: '多维度评分 + AI 详细报告 + 改进建议' }
]
</script>

<style scoped>
.home { min-height: 100vh; }

/* ===== Hero ===== */
.hero {
  position: relative;
  background: linear-gradient(135deg, #0f172a 0%, #1a2332 50%, #0f172a 100%);
  padding: 5rem 0 4rem;
  color: white;
  overflow: hidden;
}
.hero-bg {
  position: absolute; inset: 0;
  background:
    radial-gradient(circle at 20% 50%, rgba(14,165,233,0.08) 0%, transparent 50%),
    radial-gradient(circle at 80% 20%, rgba(6,182,212,0.06) 0%, transparent 50%);
}
.hero .container {
  position: relative;
  display: flex;
  align-items: center;
  gap: 4rem;
}
.hero-content { flex: 1; }
.hero-badge {
  display: inline-flex;
  align-items: center;
  gap: 0.5rem;
  background: rgba(14,165,233,0.1);
  border: 1px solid rgba(14,165,233,0.2);
  border-radius: 999px;
  padding: 0.375rem 1rem;
  font-size: 0.8125rem;
  color: #22d3ee;
  margin-bottom: 1.5rem;
}
.badge-dot {
  width: 6px; height: 6px;
  border-radius: 50%;
  background: #22d3ee;
  animation: pulse-dot 2s infinite;
}
@keyframes pulse-dot {
  0%, 100% { opacity: 1; }
  50% { opacity: 0.4; }
}
.hero-logo-wrap {
  margin-bottom: 1.5rem;
}
.hero-logo {
  width: 80px;
  height: 80px;
  border-radius: 50%;
  border: 3px solid rgba(255,255,255,0.15);
  box-shadow: 0 0 30px rgba(14, 165, 233, 0.3);
}
.hero-title {
  font-size: 3.5rem;
  font-weight: 800;
  line-height: 1.1;
  letter-spacing: -0.03em;
  margin-bottom: 1rem;
}
.hero-title .highlight {
  background: linear-gradient(135deg, #22d3ee, #0ea5e9);
  -webkit-background-clip: text;
  -webkit-text-fill-color: transparent;
  background-clip: text;
}
.hero-subtitle {
  font-size: 1.125rem;
  color: #94a3b8;
  line-height: 1.7;
  margin-bottom: 2rem;
  max-width: 520px;
}
.hero-actions {
  display: flex;
  gap: 1rem;
  margin-bottom: 3rem;
}
.hero-actions .btn-primary {
  background: linear-gradient(135deg, #0ea5e9, #06b6d4);
  display: inline-flex;
  align-items: center;
  gap: 0.5rem;
}
.btn-ghost-light {
  display: inline-flex;
  align-items: center;
  gap: 0.5rem;
  padding: 0.875rem 1.75rem;
  font-size: 1.0625rem;
  font-weight: 500;
  color: #cbd5e1;
  background: transparent;
  border: 1px solid #334155;
  border-radius: var(--radius-md);
  cursor: pointer;
  transition: all 0.2s;
}
.btn-ghost-light:hover {
  border-color: #22d3ee;
  color: #22d3ee;
  background: rgba(34,211,238,0.05);
}
.hero-stats {
  display: flex;
  align-items: center;
  gap: 2rem;
}
.stat-value {
  font-size: 1.75rem;
  font-weight: 700;
  color: #22d3ee;
  font-variant-numeric: tabular-nums;
}
.stat-label {
  font-size: 0.8125rem;
  color: #64748b;
  margin-top: 0.25rem;
}
.stat-divider {
  width: 1px;
  height: 40px;
  background: #334155;
}

/* Hero Terminal */
.hero-visual { flex: 0 0 440px; }
.terminal-card {
  background: #1e293b;
  border: 1px solid #334155;
  border-radius: 12px;
  overflow: hidden;
  box-shadow: 0 25px 50px rgba(0, 0, 0, 0.4);
}
.terminal-bar {
  display: flex;
  align-items: center;
  gap: 6px;
  padding: 10px 14px;
  background: #0f172a;
  border-bottom: 1px solid #334155;
}
.terminal-dot { width: 10px; height: 10px; border-radius: 50%; }
.terminal-dot.red { background: #ef4444; }
.terminal-dot.yellow { background: #f59e0b; }
.terminal-dot.green { background: #22c55e; }
.terminal-title {
  margin-left: 8px;
  font-size: 0.75rem;
  color: #64748b;
  font-family: 'JetBrains Mono', 'Fira Code', monospace;
}
.terminal-body {
  padding: 1rem 1.25rem;
  font-family: 'JetBrains Mono', 'Fira Code', monospace;
  font-size: 0.8125rem;
  line-height: 1.8;
}
.code-line { white-space: nowrap; }
.code-keyword { color: #c084fc; }
.code-module { color: #22d3ee; }
.code-string { color: #86efac; }
.code-var { color: #e2e8f0; }
.code-fn { color: #fbbf24; }
.code-comment { color: #475569; }

/* ===== Features ===== */
.features { padding: 5rem 0; background: #f8fafc; }
.section-title {
  font-size: 2rem; font-weight: 700; text-align: center;
  margin-bottom: 0.5rem; color: var(--text-primary);
}
.section-subtitle {
  font-size: 1rem; text-align: center;
  color: var(--text-secondary); margin-bottom: 3rem;
}
.features-grid {
  display: grid; grid-template-columns: repeat(4, 1fr); gap: 1.5rem;
}
.feature-card {
  background: white; border-radius: 12px; padding: 1.75rem;
  border: 1px solid #e2e8f0; transition: all 0.3s;
}
.feature-card:hover {
  transform: translateY(-4px);
  box-shadow: 0 12px 24px rgba(0, 0, 0, 0.06);
  border-color: #0ea5e9;
}
.feature-icon-box {
  width: 52px; height: 52px;
  border-radius: 12px;
  display: flex; align-items: center; justify-content: center;
  margin-bottom: 1rem;
}
.feature-icon { display: flex; }
.feature-title {
  font-size: 1.0625rem; font-weight: 600;
  margin-bottom: 0.5rem; color: #0f172a;
}
.feature-desc {
  font-size: 0.875rem; color: #64748b;
  line-height: 1.6; margin-bottom: 1rem;
}
.feature-tag {
  display: inline-block;
  padding: 0.2rem 0.6rem;
  font-size: 0.75rem;
  font-weight: 500;
  color: #0ea5e9;
  background: rgba(14, 165, 233, 0.08);
  border-radius: 4px;
  font-family: 'JetBrains Mono', 'Fira Code', monospace;
}

/* ===== Workflow ===== */
.workflow { padding: 5rem 0; background: white; }
.workflow-steps {
  display: flex; justify-content: space-between;
  position: relative; gap: 2rem;
}
.workflow-step {
  flex: 1; text-align: center; position: relative;
}
.step-number {
  width: 48px; height: 48px;
  background: linear-gradient(135deg, #0ea5e9, #06b6d4);
  color: white; border-radius: 12px;
  display: flex; align-items: center; justify-content: center;
  font-size: 1.25rem; font-weight: 700;
  margin: 0 auto 1rem;
  box-shadow: 0 4px 12px rgba(14, 165, 233, 0.3);
}
.step-line {
  position: absolute;
  top: 24px;
  left: calc(50% + 32px);
  width: calc(100% - 64px);
  height: 2px;
  background: #e2e8f0;
}
.step-title {
  font-size: 1rem; font-weight: 600;
  margin-bottom: 0.5rem; color: #0f172a;
}
.step-desc {
  font-size: 0.875rem; color: #64748b; line-height: 1.6;
}

/* ===== Architecture ===== */

/* ===== CTA ===== */
.cta { padding: 4rem 0; background: #f8fafc; }
.cta-box {
  background: linear-gradient(135deg, #0f172a, #1e293b);
  border-radius: 16px;
  padding: 3.5rem;
  text-align: center;
  border: 1px solid #334155;
}
.cta-title {
  font-size: 1.75rem; font-weight: 700;
  color: white; margin-bottom: 0.5rem;
}
.cta-subtitle {
  font-size: 1rem; color: #94a3b8;
  margin-bottom: 2rem;
}
.cta-box .btn-primary {
  background: linear-gradient(135deg, #0ea5e9, #06b6d4);
}

/* ===== Footer ===== */
.footer {
  padding: 2rem 0;
  background: #0f172a;
  border-top: 1px solid #1e293b;
}
.footer-inner {
  display: flex; align-items: center; justify-content: space-between;
}
.footer-brand { display: flex; align-items: center; gap: 0.75rem; }
.footer-logo {
  font-size: 1.125rem; font-weight: 700;
  color: #22d3ee;
  font-family: 'JetBrains Mono', 'Fira Code', monospace;
}
.footer-desc { font-size: 0.8125rem; color: #64748b; }
.footer-copy { font-size: 0.8125rem; color: #475569; }

/* ===== Responsive ===== */
@media (max-width: 1024px) {
  .hero .container { flex-direction: column; text-align: center; }
  .hero-visual { flex: none; width: 100%; max-width: 500px; }
  .hero-actions { justify-content: center; }
  .hero-stats { justify-content: center; }
  .hero-subtitle { margin-left: auto; margin-right: auto; }
  .features-grid { grid-template-columns: repeat(2, 1fr); }
}
@media (max-width: 640px) {
  .hero { padding: 3rem 0 2rem; }
  .hero-title { font-size: 2rem; }
  .hero-actions { flex-direction: column; }
  .hero-visual { display: none; }
  .hero-stats { flex-direction: column; gap: 1rem; }
  .stat-divider { width: 40px; height: 1px; }
  .features-grid { grid-template-columns: 1fr; }
  .workflow-steps { flex-direction: column; }
  .step-line { display: none; }
  .footer-inner { flex-direction: column; gap: 0.5rem; text-align: center; }
}
</style>
