const API_BASE_URL = 'http://localhost:8080'; // 请根据实际后端地址修改

// 通用请求方法
async function apiRequest(url, options = {}) {
    const token = localStorage.getItem('auth_token');
    
    //console.debug(token);
    const headers = {
        'Content-Type': 'application/json',
        ...options.headers,
    };

    if (token) {
        headers['Authorization'] = `Bearer ${token}`;
    }

    try {
        const response = await fetch(`${API_BASE_URL}${url}`, {
            ...options,
            headers,
        });

        if (!response.ok) {
            const error = await response.json().catch(() => ({}));
            throw new Error(error.message || `请求失败: ${response.status}`);
        }

        return await response.json();
    } catch (error) {
        console.error('API请求错误:', error);
        throw error;
    }
}

// 获取用户数据
async function fetchUserProfile() {
    const data = await apiRequest('/user/profile');
    if (data.error) {
        throw new Error(data.error);   // 抛出异常，触发重定向或提示
    }
    return {
        username: data.username,
        email: data.email,
        phone: data.phone,
        bio: data.bio,
        registerTime: data.registerTime,
        lastLogin: data.lastLogin,
        interviewCount: data.interviewCount,
        skills: data.skills,
        avatarInitial: data.username ? data.username.charAt(0) : ''
    };
}

// 获取面试历史数据 
async function fetchHistoryData() {
    try {
        const data = await apiRequest('/interview/history');
        if (Array.isArray(data) && data.length > 0) {
            return data.map(item => ({
                company: item.company,
                position: item.position,
                score: item.score,
                date: item.date,
                status: item.status,
                stage: item.stage,
                comment: item.comment,
                strengths: item.strengths,
                improvements: item.improvements
            }));
        }
        // 后端返回空数组时返回空数组
        return [];
    } catch (error) {
        console.error('获取面试历史失败:', error);
        throw error;
    }
}

// 默认面试历史数据（保留备用）
function getDefaultHistoryData() {
    return [
        { company: '阿里巴巴', position: '高级Java工程师', score: 8.5, date: '2023-04-15', status: '通过', stage: '技术面试', comment: '技术基础扎实，系统设计能力突出。', strengths: ['分布式系统理解深入', '优秀的算法能力'], improvements: ['加强特定领域技术深度', '更多业务视角'] },
        { company: '腾讯', position: '后端开发专家', score: 9.2, date: '2023-03-28', status: '通过', stage: 'HR面试', comment: '技术能力非常突出', strengths: ['快速理解复杂业务'], improvements: ['加强产品思维'] },
        { company: '字节跳动', position: '资深后端工程师', score: 6.8, date: '2023-02-10', status: '未通过', stage: '技术面试', comment: '基础良好，系统设计待加强', strengths: ['编程能力扎实'], improvements: ['加强系统设计能力'] },
        { company: '美团', position: '技术专家', score: 8.0, date: '2023-01-05', status: '通过', stage: '已入职', comment: '综合能力优秀', strengths: ['技术全面'], improvements: ['领导力培养'] }
    ];
}

// 获取分析数据 
async function fetchAnalysisData() {
    try {
        const [radarData, trendData, suggestionsData] = await Promise.all([
            apiRequest('/analysis/radar').catch(() => null),
            apiRequest('/analysis/trend').catch(() => null),
            apiRequest('/analysis/suggestions').catch(() => null)
        ]);

        if (!radarData || !trendData || !suggestionsData) {
            throw new Error('获取分析数据失败');
        }

        return {
            radar: {
                indicators: radarData.indicators,
                values: radarData.values
            },
            trend: {
                categories: trendData.categories,
                series: trendData.series
            },
            strengths: radarData.strengths,
            weaknesses: radarData.weaknesses,
            suggestions: suggestionsData.suggestions
        };
    } catch (error) {
        console.error('获取分析数据失败:', error);
        throw error;
    }
}

// 模拟后端请求 - 获取岗位详情
function fetchJobDetail(company, position) {
    return new Promise((resolve) => {
        setTimeout(() => {
            const jobData = {
                company: company,
                position: position,
                detail: `
                    <div class="job-detail">
                        <h4>${company} - ${position}</h4>
                        <div class="row mb-4">
                            <div class="col-md-6">
                                <p><strong><i class="fas fa-money-bill-wave me-2 text-primary"></i>薪资范围：</strong>30K-50K/月（14薪）</p>
                                <p><strong><i class="fas fa-map-marker-alt me-2 text-primary"></i>工作地点：</strong>北京市朝阳区</p>
                                <p><strong><i class="fas fa-clock me-2 text-primary"></i>工作经验：</strong>${position.includes('高级') || position.includes('专家') ? '5-8年' : '3-5年'}</p>
                            </div>
                            <div class="col-md-6">
                                <p><strong><i class="fas fa-tag me-2 text-primary"></i>岗位类型：</strong>全职</p>
                                <p><strong><i class="fas fa-graduation-cap me-2 text-primary"></i>学历要求：</strong>本科及以上</p>
                                <p><strong><i class="fas fa-calendar-alt me-2 text-primary"></i>发布时间：</strong>2025-06-10</p>
                            </div>
                        </div>
                        <h5><i class="fas fa-list-alt me-2 text-primary"></i>岗位职责</h5>
                        <ul class="mb-4">
                            <li>负责公司核心业务系统的架构设计和开发维护</li>
                            <li>带领技术团队完成高并发、高可用系统的迭代优化</li>
                            <li>参与技术选型和技术难点攻坚，推动技术创新</li>
                            <li>与产品、测试团队协作，保障项目交付质量和效率</li>
                        </ul>
                        <h5><i class="fas fa-check-circle me-2 text-primary"></i>任职要求</h5>
                        <ul>
                            <li>精通Java/Python/前端任意一门核心技术，有分布式系统经验</li>
                            <li>具备良好的代码规范和技术文档编写能力</li>
                            <li>有大厂同岗位经验者优先，具备团队管理经验加分</li>
                            <li>具备良好的沟通能力和团队协作精神，抗压能力强</li>
                        </ul>
                    </div>
                `,
                applyUrl: `https://example.com/apply/${company.toLowerCase()}-${position.replace(/\s+/g, '-').toLowerCase()}`
            };
            resolve(jobData);
        }, 800);
    });
}

// 添加新技能 
function addNewSkill(skill) {
    const container = document.getElementById('skills-container');
    const input = document.getElementById('new-skill-input');
    const newTag = document.createElement('span');
    newTag.className = 'badge bg-primary me-2 mb-2 skill-tag';
    newTag.innerHTML = skill + '<span class="remove-tag"><i class="fas fa-times"></i></span>';
    container.insertBefore(newTag, input);

    newTag.querySelector('.remove-tag').addEventListener('click', function (e) {
        e.stopPropagation();
        this.parentElement.remove();
    });
}

// 初始化技能标签
async function initSkillTags() {
    const user = await fetchUserProfile();
    if (!user.skills || !Array.isArray(user.skills)) {
        user.skills = [];
    }
    const container = document.getElementById('skills-container');
    const input = document.getElementById('new-skill-input');

    container.innerHTML = '';
    user.skills.forEach(skill => {
        const tag = document.createElement('span');
        tag.className = 'badge bg-primary me-2 mb-2 skill-tag';
        tag.innerHTML = skill + '<span class="remove-tag"><i class="fas fa-times"></i></span>';
        container.appendChild(tag);

        tag.querySelector('.remove-tag').addEventListener('click', function (e) {
            e.stopPropagation();
            this.parentElement.remove();
        });
    });
    container.appendChild(input);

    document.getElementById('add-skill-btn').addEventListener('click', function () {
        input.style.display = 'inline-block';
        input.focus();
    });

    input.addEventListener('keypress', function (e) {
        if (e.key === 'Enter' && this.value.trim()) {
            addNewSkill(this.value.trim());
            this.value = '';
            this.style.display = 'none';
        }
    });

    document.addEventListener('click', function (e) {
        if (e.target !== input && e.target.id !== 'add-skill-btn') {
            input.style.display = 'none';
        }
    });
}

// 个人信息编辑
async function initProfileEditing() {
    const user = await fetchUserProfile();
    document.getElementById('username-text').innerText = user.username;
    document.getElementById('email-text').innerText = user.email;
    document.getElementById('phone-text').innerText = user.phone;
    document.getElementById('bio-text').innerText = user.bio;
    document.getElementById('user-name-display').innerText = user.username;
    document.getElementById('register-time').innerText = user.registerTime;
    document.getElementById('last-login').innerText = user.lastLogin;
    document.getElementById('interview-count').innerText = user.interviewCount;
    document.getElementById('user-avatar').innerText = user.avatarInitial;

    document.getElementById('avatar-input').addEventListener('change', function (e) {
        const file = e.target.files[0];
        if (file) {
            const reader = new FileReader();
            reader.onload = function (event) {
                const avatar = document.getElementById('user-avatar');
                avatar.style.backgroundImage = `url(${event.target.result})`;
                avatar.style.backgroundSize = 'cover';
                avatar.style.backgroundPosition = 'center';
                avatar.textContent = '';
            };
            reader.readAsDataURL(file);
        }
    });

    document.querySelectorAll('.edit-icon').forEach(icon => {
        icon.addEventListener('click', function (e) {
            e.stopPropagation();
            const field = this.getAttribute('data-field');
            const textElement = document.getElementById(field + '-text');
            const inputElement = document.getElementById(field + '-input');
            textElement.style.display = 'none';
            inputElement.style.display = 'block';
            inputElement.focus();
        });
    });

    document.querySelectorAll('.edit-input').forEach(input => {
        input.addEventListener('blur', function () {
            const field = this.id.replace('-input', '');
            const textElement = document.getElementById(field + '-text');
            textElement.textContent = this.value;
            if (field === 'username') document.getElementById('user-name-display').textContent = this.value;
            this.style.display = 'none';
            textElement.style.display = 'inline';
        });
    });
}

// 简历编辑
function initResumeEditing() {
    document.getElementById('export-pdf-btn').addEventListener('click', () => alert('导出PDF功能开发中'));
    document.getElementById('reset-resume-btn').addEventListener('click', function () {
        if (confirm('确定要重置简历内容吗？')) {
            document.getElementById('resume-content').innerHTML = `
                <h2>张三</h2><p>高级软件工程师 | 5年经验</p><hr>
                <h3>联系方式</h3><p>电话: 13800138000 | 邮箱: zhangsan@example.com</p>
                <h3>教育背景</h3><p><strong>清华大学</strong> - 计算机科学与技术 (2014-2018)</p>
                <h3>工作经历</h3><p><strong>ABC科技有限公司</strong> - 高级软件工程师 (2018-至今)</p>
                <h3>专业技能</h3><ul><li>精通Java、Python编程</li><li>熟悉Spring Boot、React等框架</li></ul>
            `;
        }
    });
    document.getElementById('save-resume-btn').addEventListener('click', () => alert('简历保存成功！'));
}

// 初始化申请按钮事件
function initApplyJobButton() {
    document.querySelectorAll('.apply-job-btn').forEach(btn => {
        btn.addEventListener('click', async function (e) {
            e.stopPropagation();

            const company = this.getAttribute('data-company');
            const position = this.getAttribute('data-position');
            const modalContent = document.getElementById('jobDetailContent');

            modalContent.innerHTML = `
                <div class="text-center py-5">
                    <div class="spinner-border text-primary" role="status">
                        <span class="visually-hidden">加载中...</span>
                    </div>
                    <p class="mt-2 text-muted">正在加载岗位详情...</p>
                </div>
            `;

            try {
                const jobData = await fetchJobDetail(company, position);
                modalContent.innerHTML = jobData.detail;
                document.getElementById('jobApplyLink').href = jobData.applyUrl;
                const modal = new bootstrap.Modal(document.getElementById('jobApplyModal'));
                modal.show();
            } catch (error) {
                modalContent.innerHTML = `
                    <div class="alert alert-danger text-center py-4">
                        <i class="fas fa-exclamation-triangle me-2 fs-4"></i>
                        <p class="mb-0">加载岗位详情失败，请稍后重试</p>
                    </div>
                `;
            }
        });
    });
}

// 全局图表实例
let radarChart = null;
let trendChart = null;

// 初始化雷达图
function initRadarChart(data) {
    const container = document.getElementById('radarChart');
    if (!container) return;

    if (radarChart) {
        radarChart.dispose();
    }

    radarChart = echarts.init(container);

    const option = {
        title: {
            text: '能力素质雷达图',
            left: 'center',
            top: 15,
            textStyle: { fontSize: 15, fontWeight: 'normal', color: '#333' }
        },
        tooltip: { trigger: 'item' },
        radar: {
            indicator: data.radar.indicators.map(name => ({ name, max: 10 })),
            center: ['50%', '52%'],
            radius: '70%',
            name: {
                textStyle: { fontSize: 11, fontWeight: 500, color: '#2c3e66' }
            },
            splitArea: {
                areaStyle: { color: ['rgba(26,115,232,0.02)', 'rgba(66,133,244,0.05)'] }
            },
            axisLine: { lineStyle: { color: '#cbd5e1' } },
            axisTick: { show: true }
        },
        series: [{
            type: 'radar',
            data: [{ value: data.radar.values, name: '当前能力' }],
            areaStyle: { color: 'rgba(26,115,232,0.25)' },
            lineStyle: { color: '#1a73e8', width: 2.5 },
            symbol: 'circle',
            symbolSize: 7,
            itemStyle: { color: '#0d47a1', borderColor: '#fff', borderWidth: 2 }
        }]
    };

    radarChart.setOption(option);

    setTimeout(() => {
        if (radarChart) radarChart.resize();
    }, 100);

    window.addEventListener('resize', () => {
        if (radarChart) radarChart.resize();
    });
}

// 初始化趋势图
function initTrendChart(data) {
    const container = document.getElementById('trendChart');
    if (!container) return;

    if (trendChart) {
        trendChart.dispose();
    }

    trendChart = echarts.init(container);

    const option = {
        title: {
            text: '能力变化趋势',
            left: 'center',
            top: 10,
            textStyle: { fontSize: 14, fontWeight: 'normal' }
        },
        tooltip: { trigger: 'axis' },
        grid: {
            left: '8%',
            right: '5%',
            top: '18%',
            bottom: '8%',
            containLabel: true
        },
        xAxis: {
            type: 'category',
            data: data.trend.categories,
            axisLabel: { fontSize: 12, fontWeight: 500 }
        },
        yAxis: {
            type: 'value',
            min: 5,
            max: 10,
            name: '能力评分',
            axisLabel: { fontSize: 11 }
        },
        series: data.trend.series.map(s => ({
            name: s.name,
            type: 'line',
            data: s.data,
            smooth: true,
            lineStyle: { color: '#1a73e8', width: 3 },
            symbol: 'circle',
            symbolSize: 9,
            areaStyle: { color: 'rgba(26,115,232,0.08)' },
            itemStyle: { color: '#1a73e8', borderColor: '#fff', borderWidth: 2 }
        }))
    };

    trendChart.setOption(option);

    setTimeout(() => {
        if (trendChart) trendChart.resize();
    }, 100);

    window.addEventListener('resize', () => {
        if (trendChart) trendChart.resize();
    });
}

// 渲染优势和待提升能力
function renderStrengthsWeaknesses(data) {
    const strengthList = document.getElementById('strength-list');
    const weaknessList = document.getElementById('weakness-list');
    const suggestionDiv = document.getElementById('suggestion-content');

    strengthList.innerHTML = `<ul class="list-group list-group-flush">${data.strengths.map(s => `
        <li class="list-group-item d-flex justify-content-between align-items-center">
            ${s.name}
            <div><span class="badge bg-primary rounded-pill me-2">${s.score}</span><span class="text-success"><i class="fas fa-arrow-up"></i> ${s.trend}</span></div>
        </li>
    `).join('')}</ul>`;

    weaknessList.innerHTML = `<ul class="list-group list-group-flush">${data.weaknesses.map(w => `
        <li class="list-group-item d-flex justify-content-between align-items-center">
            ${w.name}
            <div><span class="badge bg-primary rounded-pill me-2">${w.score}</span><span class="${w.trend.startsWith('+') ? 'text-success' : 'text-danger'}"><i class="fas ${w.trend.startsWith('+') ? 'fa-arrow-up' : 'fa-arrow-down'}"></i> ${w.trend}</span></div>
        </li>
    `).join('')}</ul>`;

    suggestionDiv.innerHTML = `
        <div class="alert alert-info mb-2">
            <i class="fas fa-lightbulb me-2"></i>根据您的面试表现，我们为您准备了以下提升建议：
        </div>
        <ul>${data.suggestions.map(s => `<li class="mb-2">${s}</li>`).join('')}</ul>
    `;
}

// 历史记录
async function initHistorySection() {
    const histories = await fetchHistoryData();
    const historyContainer = document.getElementById('history-list-container');
    document.getElementById('total-history').innerText = histories.length;

    historyContainer.innerHTML = histories.map(h => `
        <div class="history-item">
            <div class="history-bar">
                <div class="history-score ${h.score >= 8 ? 'bg-success' : (h.score >= 6 ? 'bg-warning' : 'bg-danger')}">${h.score}</div>
                <div class="history-content">
                    <div class="history-title">${h.company} - ${h.position}</div>
                    <div class="history-date">${h.date}</div>
                </div>
                <button class="btn apply-job-btn" 
                        data-company="${h.company}" 
                        data-position="${h.position}">
                    <i class="fas fa-paper-plane me-1"></i> 申请
                </button>
                <div class="history-toggle"><i class="fas fa-chevron-down"></i></div>
            </div>
            <div class="history-details">
                <div class="mb-2"><span class="badge ${h.status === '通过' ? 'bg-success' : 'bg-danger'} me-2">${h.status}</span><span class="text-muted">${h.stage}</span></div>
                <h6>面试评价:</h6><p>${h.comment}</p>
                <h6>技术亮点:</h6><ul>${h.strengths.map(s => `<li>${s}</li>`).join('')}</ul>
                <h6>改进建议:</h6><ul>${h.improvements.map(i => `<li>${i}</li>`).join('')}</ul>
            </div>
        </div>
    `).join('');

    document.querySelectorAll('.history-bar').forEach(bar => {
        bar.addEventListener('click', function () {
            const item = this.closest('.history-item');
            item.classList.toggle('expanded');
        });
    });

    document.getElementById('search-history-btn').addEventListener('click', function () {
        const searchTerm = document.getElementById('history-search').value.toLowerCase();
        document.querySelectorAll('.history-item').forEach(item => {
            item.style.display = item.textContent.toLowerCase().includes(searchTerm) ? 'block' : 'none';
        });
    });

    document.getElementById('load-more-history').addEventListener('click', () => alert('加载更多功能开发中'));

    initApplyJobButton();
}

// 页面初始化
document.addEventListener('DOMContentLoaded', async function () {
    if (!localStorage.getItem('auth_token')) {
        window.location.href = '/login.html';
        return;
    }

    try {
        // 初始化各模块
        await initProfileEditing();
        await initSkillTags();
        initResumeEditing();
        await initHistorySection();

        // 初始化图表
        const analysisData = await fetchAnalysisData();
        initRadarChart(analysisData);
        initTrendChart(analysisData);
        renderStrengthsWeaknesses(analysisData);

        // 时间范围切换
        document.querySelectorAll('.time-range-btn, #time-range-select').forEach(el => {
            el.addEventListener('click', async function () {
                const newData = await fetchAnalysisData();
                initRadarChart(newData);
                initTrendChart(newData);
                renderStrengthsWeaknesses(newData);
            });
        });

        // 导航切换
        document.querySelectorAll('.nav-item').forEach(item => {
            item.addEventListener('click', function () {
                document.querySelectorAll('.nav-item').forEach(nav => nav.classList.remove('active'));
                this.classList.add('active');

                document.querySelectorAll('.content-section').forEach(section => {
                    section.classList.remove('active');
                });

                const sectionId = this.getAttribute('data-section');
                document.getElementById(sectionId).classList.add('active');

                setTimeout(() => {
                    if (radarChart) radarChart.resize();
                    if (trendChart) trendChart.resize();
                }, 100);
            });
        });
    } catch (error) {
        console.error('页面初始化失败:', error);
        alert('加载数据失败，请刷新页面重试');
    }
});