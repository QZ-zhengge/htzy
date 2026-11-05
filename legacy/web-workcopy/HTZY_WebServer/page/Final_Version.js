let industryRegions = {
    "all": [
        "全国",
        "北京", "天津", "河北", "山西", "内蒙古",
        "辽宁", "吉林", "黑龙江",
        "上海", "江苏", "浙江", "安徽", "福建", "江西", "山东",
        "河南", "湖北", "湖南", "广东", "广西", "海南",
        "重庆", "四川", "贵州", "云南", "西藏",
        "陕西", "甘肃", "青海", "宁夏", "新疆",
        "香港", "澳门"
    ]
};
let jobsData = [];

// ========== 红色指引模块数据 ==========
// 这些将作为后备数据，优先从后端获取
let industries = ["互联网/IT", "金融", "教育培训", "医疗健康", "制造业", "红色文旅"];
let regions = ["北京", "上海", "广州", "深圳", "杭州", "成都"];
let demoJobsData = [
    { title: "前端开发工程师", company: "红色基因科技", location: "北京", salary: "20-35K", industry: "互联网" },
    { title: "红色文旅策划", company: "星火文化传媒", location: "延安", salary: "10-15K", industry: "文旅" },
    { title: "基层党务助理", company: "乡村振兴促进会", location: "江西井冈山", salary: "6-9K", industry: "公共事业" },
    { title: "Java后端工程师", company: "浪潮云", location: "济南", salary: "18-28K", industry: "互联网" },
    { title: "红色教育讲师", company: "红源干部学院", location: "遵义", salary: "12-18K", industry: "教育" },
    { title: "数据分析师", company: "数智红途", location: "杭州", salary: "16-25K", industry: "互联网" }
];

// 红色指引模块动态数据（将从后端获取）
let carouselItems = [
    { image: "https://picsum.photos/id/1002/1200/450", title: "红色精神 科技报国", description: "从“两弹一星”到国产芯片，计算机事业在红色指引下蓬勃发展" },
    { image: "https://picsum.photos/id/1015/1200/450", title: "乡村振兴 数字赋能", description: "数字化基层就业，为乡村振兴插上科技翅膀" },
    { image: "https://picsum.photos/id/1039/1200/450", title: "红色教育基地 · 职业成长", description: "联合红色遗址与高校，打造计算机科技人才培养计划" }
];

let storyCards = [
    { icon: "fa-microchip", title: "红色计算机记忆", content: "从1956年计算所诞生到国产操作系统突破，一代代科研人员赓续红色血脉，突破技术封锁。" },
    { icon: "fa-network-wired", title: "网信事业初心", content: "坚持自主创新，构建网络安全防线，红色指引下数字中国战略稳步推进。" },
    { icon: "fa-chalkboard-user", title: "基层数字化帮扶", content: "开展乡村电商、智慧农业培训，以信息技术助力老区振兴。" },
    { icon: "fa-hand-holding-heart", title: "退役军人IT扶持", content: "联合龙头企业提供编程、运维等定向培训，护航退役军人职业转型。" },
    { icon: "fa-people-arrows", title: "红色企业对接", content: "汇聚信创、智慧党建等领域优质岗位，搭建精准就业桥梁。" },
    { icon: "fa-chart-line", title: "红色职业规划", content: "传承艰苦奋斗精神，为青年提供长期技术成长路线与价值观引导。" }
];

// 红色指引模块全局变量
let currentSlideIdx = 0;
let slideCounts = carouselItems.length;
let autoTimer = null;

// ========== 页面切换核心逻辑 ==========
// 兼容新版 HTML（homePageContainer 已内联在页面，redGuidePage 为 section#redGuidePage）
// 旧版保留：homePageContainer / redGuideModule；新版：各首页 section + section#redGuidePage
function getHomeContainers() {
    // 新版：首页由多个 section 组成，用一个包装 div 控制；
    // 如果没有 homePageContainer，则找 section.hero / section.filters 等首页区块
    const wrap = document.getElementById("homePageContainer");
    if (wrap) return [wrap];
    return Array.from(document.querySelectorAll(
        "section.hero, section.filters, section.cities, section.container, .modal-overlay"
    )).filter(el => el.id !== "redGuidePage");
}
function getRedModule() {
    return document.getElementById("redGuidePage") || document.getElementById("redGuideModule");
}

const navHomeLink = document.getElementById("navHome");
const navRedLink  = document.getElementById("toRedGuide") || document.getElementById("navRedGuide");
const goHomeLogo  = document.getElementById("goHome") || document.getElementById("goHomeBtn");

function setActiveNav(isRedActive) {
    if (navRedLink) {
        navRedLink.style.color = isRedActive ? "#dc2626" : "";
        navRedLink.style.fontWeight = isRedActive ? "700" : "";
    }
    if (navHomeLink) {
        navHomeLink.style.color = isRedActive ? "" : "#dc2626";
    }
}

function destroyAutoPlay() {
    if (autoTimer) clearInterval(autoTimer);
    autoTimer = null;
}

// 首页各 section 的 display 值（因 CSS 里 section 默认 block）
const HOME_SECTIONS_DISPLAY = "block";

function showHome() {
    const redEl = getRedModule();
    if (redEl) redEl.style.display = "none";
    getHomeContainers().forEach(el => { el.style.display = HOME_SECTIONS_DISPLAY; });
    setActiveNav(false);
    destroyAutoPlay();
}

let _pdfCardsRendered = false; // PDF 卡片只渲染一次

function showRedGuide() {
    getHomeContainers().forEach(el => { el.style.display = "none"; });
    const redEl = getRedModule();
    if (redEl) redEl.style.display = "block";
    setActiveNav(true);
    currentSlideIdx = 0;
    renderCarousel();
    renderStories();
    rebindCarouselEvents();
    // 首次显示时加载 PDF 卡片
    if (!_pdfCardsRendered) {
        _pdfCardsRendered = true;
        fetchAndRenderPdfCards();
    }
    const carouselWrapper = document.querySelector("#redGuidePage .carousel-container")
        || document.querySelector("#redGuideModule .carousel-container");
    if (carouselWrapper) {
        carouselWrapper.removeEventListener("mouseenter", stopAutoPlay);
        carouselWrapper.removeEventListener("mouseleave", startAutoPlay);
        carouselWrapper.addEventListener("mouseenter", stopAutoPlay);
        carouselWrapper.addEventListener("mouseleave", startAutoPlay);
    }
    startAutoPlay();
}

// ========== 红色指引模块函数 ==========
function escapeHtml(str) {
    if (!str) return "";
    return str.replace(/[&<>]/g, function (m) {
        if (m === '&') return '&amp;';
        if (m === '<') return '&lt;';
        if (m === '>') return '&gt;';
        return m;
    });
}

function renderCarousel() {
    // 兼容两套 HTML 结构中的 ID：新版用 carouselSlides / carouselIndicators，旧版用 redCarouselSlides / redIndicators
    const slidesContainer = document.getElementById("carouselSlides") || document.getElementById("redCarouselSlides");
    const indicatorsContainer = document.getElementById("carouselIndicators") || document.getElementById("redIndicators");
    if (!slidesContainer || !indicatorsContainer) return;
    slidesContainer.innerHTML = "";
    indicatorsContainer.innerHTML = "";
    if (!carouselItems.length) {
        slidesContainer.innerHTML = '<div class="carousel-slide" style="display:flex;align-items:center;justify-content:center;">暂无数据</div>';
        return;
    }
    carouselItems.forEach((item, idx) => {
        const slideDiv = document.createElement("div");
        slideDiv.className = "carousel-slide";
        slideDiv.innerHTML = `
            <img src="${item.image}" alt="${escapeHtml(item.title)}" onerror="this.src='https://picsum.photos/id/100/1200/450'">
            <div class="carousel-caption">
                <h3>${escapeHtml(item.title)}</h3>
                <p>${escapeHtml(item.description)}</p>
            </div>
        `;
        slidesContainer.appendChild(slideDiv);

        const indicator = document.createElement("div");
        indicator.className = `indicator ${idx === 0 ? "active" : ""}`;
        indicator.dataset.index = idx;
        indicator.addEventListener("click", () => goToSlide(idx));
        indicatorsContainer.appendChild(indicator);
    });
    updateCarouselPosition();
}

function renderStories() {
    // 兼容两套 HTML 结构中的 ID：新版用 storiesContainer，旧版用 redStoriesGrid
    const storiesContainer = document.getElementById("storiesContainer") || document.getElementById("redStoriesGrid");
    if (!storiesContainer) return;
    storiesContainer.innerHTML = "";
    storyCards.forEach(story => {
        const card = document.createElement("div");
        card.className = "guide-card";
        card.innerHTML = `
            <div class="guide-card-icon">
                <i class="fas ${story.icon}"></i>
            </div>
            <h3>${escapeHtml(story.title)}</h3>
            <p>${escapeHtml(story.content)}</p>
        `;
        storiesContainer.appendChild(card);
    });
}

// ========== PDF 资料列表渲染 ==========
async function fetchAndRenderPdfCards() {
    const container = document.getElementById("storiesContainer") || document.getElementById("redStoriesGrid");
    if (!container) return;

    let pdfList = [];
    try {
        const res = await fetch("http://localhost:8080/api/redguide/pdfs");
        if (res.ok) {
            const data = await res.json();
            if (data.data && data.data.length > 0) {
                pdfList = data.data; // [{name, url, title}, ...]
            }
        }
    } catch (err) {
        console.log("PDF 列表获取失败，仅显示默认卡片：", err);
    }

    if (pdfList.length === 0) return; // 没有 PDF 则保留默认故事卡片

    // 在默认故事卡片区下方追加 PDF 资料卡片
    const pdfSection = document.createElement("div");
    pdfSection.id = "pdfCardsSection";
    pdfSection.style.cssText = "width:100%; margin-top: 40px;";
    pdfSection.innerHTML = `
        <h2 class="section-title" style="color:#b91c1c;text-align:center;margin-bottom:24px;">
            <i class="fas fa-file-pdf"></i> 红色学习资料
        </h2>
        <div class="content-grid" id="pdfCardsGrid"></div>
    `;
    container.parentNode.insertBefore(pdfSection, container.nextSibling);

    const pdfGrid = document.getElementById("pdfCardsGrid");
    const icons = ["fa-flag", "fa-star", "fa-book-open", "fa-monument", "fa-seedling", "fa-heart"];
    pdfList.forEach((pdf, idx) => {
        const card = document.createElement("div");
        card.className = "guide-card pdf-card";
        card.style.cursor = "pointer";
        const icon = icons[idx % icons.length];
        // 对 URL 的文件名部分进行编码，支持中文文件名
        const urlParts = pdf.url.split('/');
        const encodedUrl = urlParts.map((part, i) => i < 3 ? part : encodeURIComponent(part)).join('/');
        const displayTitle = escapeHtml(pdf.title || pdf.name);

        card.innerHTML = `
            <div class="guide-card-icon">
                <i class="fas ${icon}"></i>
            </div>
            <h3>${displayTitle}</h3>
            <p style="color:#6b7280; font-size:13px; margin-bottom:12px;">
                <i class="fas fa-file-pdf" style="color:#ef4444;"></i> PDF 文件
            </p>
            <div style="display:flex; gap:10px; margin-top:8px;">
                <button class="btn btn-primary" style="padding:6px 16px;font-size:13px;" data-pdf-url="${encodedUrl}" data-pdf-title="${displayTitle}">
                    <i class="fas fa-eye"></i> 查看
                </button>
                <a href="${encodedUrl}" download class="btn btn-outline" style="padding:6px 16px;font-size:13px;text-decoration:none;">
                    <i class="fas fa-download"></i> 下载
                </a>
            </div>
        `;
        // 用 addEventListener 代替 inline onclick，避免引号嵌套问题
        const viewBtn = card.querySelector('button[data-pdf-url]');
        if (viewBtn) {
            viewBtn.addEventListener('click', function() {
                openPdfModal(this.getAttribute('data-pdf-url'), this.getAttribute('data-pdf-title'));
            });
        }
        pdfGrid.appendChild(card);
    });
}

// PDF 弹窗：打开
function openPdfModal(encodedUrl, title) {
    // encodedUrl 已经是 encodeURIComponent 编码过的，浏览器可直接使用
    const modal = document.getElementById("pdfViewerModal");
    const iframe = document.getElementById("pdfViewerFrame");
    const titleEl = document.getElementById("pdfViewerTitle");
    if (!modal || !iframe) return;
    if (titleEl) titleEl.textContent = title || "文档预览";
    // 使用浏览器原生 PDF 渲染，图片和文字均可正常显示
    iframe.src = encodedUrl;
    modal.style.display = "flex";
    document.body.style.overflow = "hidden";
}

// PDF 弹窗：关闭
function closePdfModal() {
    const modal = document.getElementById("pdfViewerModal");
    const iframe = document.getElementById("pdfViewerFrame");
    if (iframe) iframe.src = "";
    if (modal) modal.style.display = "none";
    document.body.style.overflow = "";
}

function updateCarouselPosition() {
    const slidesContainer = document.getElementById("carouselSlides") || document.getElementById("redCarouselSlides");
    if (!slidesContainer) return;
    slidesContainer.style.transform = `translateX(-${currentSlideIdx * 100}%)`;
    const indicatorWrap = document.getElementById("carouselIndicators") || document.getElementById("redIndicators");
    if (!indicatorWrap) return;
    const indicators = indicatorWrap.querySelectorAll(".indicator");
    indicators.forEach((ind, i) => {
        if (i === currentSlideIdx) ind.classList.add("active");
        else ind.classList.remove("active");
    });
}

function goToSlide(index) {
    if (slideCounts === 0) return;
    if (index < 0) index = slideCounts - 1;
    if (index >= slideCounts) index = 0;
    currentSlideIdx = index;
    updateCarouselPosition();
}

function nextSlide() { goToSlide(currentSlideIdx + 1); }
function prevSlide() { goToSlide(currentSlideIdx - 1); }

function startAutoPlay() {
    if (autoTimer) clearInterval(autoTimer);
    if (slideCounts > 1) {
        autoTimer = setInterval(() => nextSlide(), 5000);
    }
}

function stopAutoPlay() {
    if (autoTimer) {
        clearInterval(autoTimer);
        autoTimer = null;
    }
}

function rebindCarouselEvents() {
    // 兼容新版 ID (prevBtn/nextBtn) 和旧版 ID (redPrevBtn/redNextBtn)
    let prevBtn = document.getElementById("prevBtn") || document.getElementById("redPrevBtn");
    let nextBtn = document.getElementById("nextBtn") || document.getElementById("redNextBtn");
    if (prevBtn) {
        const newPrev = prevBtn.cloneNode(true);
        prevBtn.parentNode.replaceChild(newPrev, prevBtn);
        prevBtn = newPrev;
    }
    if (nextBtn) {
        const newNext = nextBtn.cloneNode(true);
        nextBtn.parentNode.replaceChild(newNext, nextBtn);
        nextBtn = newNext;
    }
    const freshPrev = document.getElementById("prevBtn") || document.getElementById("redPrevBtn");
    const freshNext = document.getElementById("nextBtn") || document.getElementById("redNextBtn");
    if (freshPrev) freshPrev.addEventListener("click", () => { goToSlide(currentSlideIdx - 1); });
    if (freshNext) freshNext.addEventListener("click", () => { goToSlide(currentSlideIdx + 1); });

    const indicatorWrap = document.getElementById("carouselIndicators") || document.getElementById("redIndicators");
    if (indicatorWrap) {
        const indicators = indicatorWrap.querySelectorAll(".indicator");
        indicators.forEach((ind, idx) => {
            ind.removeEventListener("click", () => goToSlide(idx));
            ind.addEventListener("click", () => goToSlide(idx));
        });
    }
}

function initRedGuideModule() {
    renderCarousel();
    renderStories();
    rebindCarouselEvents();
    // 兼容新版容器 ID（redGuidePage）和旧版（redGuideModule）
    const carouselWrapper = document.querySelector("#redGuidePage .carousel-container")
        || document.querySelector("#redGuideModule .carousel-container");
    if (carouselWrapper) {
        carouselWrapper.removeEventListener("mouseenter", stopAutoPlay);
        carouselWrapper.removeEventListener("mouseleave", startAutoPlay);
        carouselWrapper.addEventListener("mouseenter", stopAutoPlay);
        carouselWrapper.addEventListener("mouseleave", startAutoPlay);
    }
    startAutoPlay();
}

// ========== 红色指引后端数据获取函数（新增） ==========
async function fetchRedGuideData() {
    console.log("[RedGuide] 开始从后端获取数据...");
    try {
        // 获取轮播图数据
        const carouselRes = await fetch("http://localhost:8080/api/redguide/carousel");
        console.log("[RedGuide] carousel 状态:", carouselRes.status);
        if (carouselRes.ok) {
            const carouselData = await carouselRes.json();
            console.log("[RedGuide] carousel 数据:", carouselData);
            if (carouselData.data && carouselData.data.length > 0) {
                carouselItems = carouselData.data;
                slideCounts = carouselItems.length;
            }
        }
    } catch (err) {
        console.log("轮播图数据获取失败，使用默认数据:", err);
    }

    try {
        // 获取故事卡片数据
        const storiesRes = await fetch("http://localhost:8080/api/redguide/stories");
        console.log("[RedGuide] stories 状态:", storiesRes.status);
        if (storiesRes.ok) {
            const storiesData = await storiesRes.json();
            console.log("[RedGuide] stories 数据:", storiesData);
            if (storiesData.data && storiesData.data.length > 0) {
                storyCards = storiesData.data;
            }
        }
    } catch (err) {
        console.log("故事卡片数据获取失败，使用默认数据:", err);
    }

    try {
        // 获取首页演示行业数据
        const industriesRes = await fetch("http://localhost:8080/api/redguide/industries");
        console.log("[RedGuide] industries 状态:", industriesRes.status);
        if (industriesRes.ok) {
            const industriesData = await industriesRes.json();
            console.log("[RedGuide] industries 数据:", industriesData);
            if (industriesData.data && industriesData.data.length > 0) {
                industries = industriesData.data;
            }
        }
    } catch (err) {
        console.log("行业数据获取失败，使用默认数据:", err);
    }

    try {
        // 获取首页演示地区数据
        const regionsRes = await fetch("http://localhost:8080/api/redguide/regions");
        console.log("[RedGuide] regions 状态:", regionsRes.status);
        if (regionsRes.ok) {
            const regionsData = await regionsRes.json();
            console.log("[RedGuide] regions 数据:", regionsData);
            if (regionsData.data && regionsData.data.length > 0) {
                regions = regionsData.data;
            }
        }
    } catch (err) {
        console.log("地区数据获取失败，使用默认数据:", err);
    }

    try {
        // 获取首页演示职位数据
        const demoJobsRes = await fetch("http://localhost:8080/api/redguide/demo-jobs");
        console.log("[RedGuide] demo-jobs 状态:", demoJobsRes.status);
        if (demoJobsRes.ok) {
            const demoJobsResp = await demoJobsRes.json();
            console.log("[RedGuide] demo-jobs 数据:", demoJobsResp);
            if (demoJobsResp.data && demoJobsResp.data.length > 0) {
                // 清空原有数据并填充新数据
                demoJobsData.length = 0;
                demoJobsData.push(...demoJobsResp.data);
            }
        }
    } catch (err) {
        console.log("演示职位数据获取失败，使用默认数据:", err);
    }
    console.log("[RedGuide] 数据获取完毕");
}

// ========== 首页演示数据填充（支持后端数据） ==========
function initHomeDemoData() {
    const indusContainer = document.getElementById("demoIndustries");
    if (indusContainer) {
        indusContainer.innerHTML = '';
        industries.forEach(ind => {
            let span = document.createElement("span");
            span.className = "filter-option";
            span.innerText = ind;
            indusContainer.appendChild(span);
        });
    }
    const regionContainer = document.getElementById("demoRegions");
    if (regionContainer) {
        regionContainer.innerHTML = '';
        regions.forEach(reg => {
            let span = document.createElement("span");
            span.className = "filter-option";
            span.innerText = reg;
            regionContainer.appendChild(span);
        });
    }
    const jobsGrid = document.getElementById("jobsDemoGrid");
    if (jobsGrid) {
        jobsGrid.innerHTML = '';
        demoJobsData.forEach(job => {
            const card = document.createElement("div");
            card.className = "job-card";
            card.innerHTML = `
                <div class="job-title">${job.title}</div>
                <div style="margin: 8px 0; color:#4b5563;">${job.company}</div>
                <div style="display:flex; justify-content:space-between; margin-top: 12px;">
                    <span><i class="fas fa-map-marker-alt"></i> ${job.location}</span>
                    <span class="job-salary">${job.salary}</span>
                </div>
            `;
            jobsGrid.appendChild(card);
        });
    }
}

// ========== Final_Version 核心函数（完全保持不变） ==========
function renderIndustryOptions() {
    const industryContainer = document.getElementById('industry-options');
    industryContainer.innerHTML = '';

    Object.keys(industryRegions).forEach((key, index) => {
        const div = document.createElement('div');
        div.className = `filter-option ${index === 0 ? 'active' : ''}`;
        div.setAttribute('data-industry', key);
        div.textContent = key === 'all' ? '全部' : key;
        div.addEventListener('click', () => {
            document.querySelectorAll('#industry-options .filter-option')
                .forEach(opt => opt.classList.remove('active'));
            div.classList.add('active');
            updateRegions(key);
            renderJobs(key);
        });
        industryContainer.appendChild(div);
    });
}

function updateRegions(industry) {
    const regionsContainer = document.getElementById('region-options');
    regionsContainer.innerHTML = '';

    (industryRegions[industry] || []).forEach((region, i) => {
        const regionOption = document.createElement('div');
        regionOption.className = `filter-option region-option ${i === 0 ? 'active' : ''}`;
        regionOption.textContent = region;
        regionOption.addEventListener('click', () => {
            document.querySelectorAll('#region-options .filter-option')
                .forEach(opt => opt.classList.remove('active'));
            regionOption.classList.add('active');
            filterJobsByRegion(region);
        });
        regionsContainer.appendChild(regionOption);
    });
}

function renderJobs(industry) {
    const jobsGrid = document.getElementById('jobs-container');
    jobsGrid.innerHTML = '';
    const filtered = industry === 'all' ? jobsData : jobsData.filter(j => j.industry === industry);
    filtered.forEach(job => jobsGrid.appendChild(createJobCard(job)));
}

function filterJobsByRegion(region) {
    const industry = document.querySelector('#industry-options .active').getAttribute('data-industry');
    const jobsGrid = document.getElementById('jobs-container');
    jobsGrid.innerHTML = '';
    let filtered = industry === 'all' ? jobsData : jobsData.filter(j => j.industry === industry);
    if (region !== '全国') filtered = filtered.filter(j => j.location === region);
    filtered.forEach(job => jobsGrid.appendChild(createJobCard(job)));
}

function createJobCard(job) {
    // 字段空值防御
    const title = job.title || '职位信息';
    const company = job.company || '';
    const companyLogo = company ? company.substring(0, 2) : '企';
    const location = job.location || '-';
    const experience = job.experience || '-';
    const education = job.education || '-';
    const salary = job.salary || '面议';
    const tags = Array.isArray(job.tags) ? job.tags : [];

    const div = document.createElement('div');
    div.className = 'job-card';
    div.innerHTML = `
    <div class="job-header">
      <h3 class="job-title">${title}</h3>
      <div class="company">
        <div class="company-logo">${companyLogo}</div>
        <div class="company-name">${company}</div>
      </div>
    </div>
    <div class="job-details">
      <div class="job-info">
        <div class="job-info-item"><i class="fas fa-map-marker-alt"></i> <span>${location}</span></div>
        <div class="job-info-item"><i class="fas fa-briefcase"></i> <span>${experience}</span></div>
        <div class="job-info-item"><i class="fas fa-graduation-cap"></i> <span>${education}</span></div>
      </div>
      <div class="job-tags">
        ${tags.map(t => `<span class="job-tag">${t}</span>`).join('')}
      </div>
      <div class="job-actions">
        <div class="job-salary">${salary}</div>
        <button class="btn btn-primary apply-btn" style="padding:5px 15px;font-size:14px;" data-job='${JSON.stringify(job).replace(/'/g, "&#39;")}'>申请</button>
      </div>
    </div>`;

    const applyBtn = div.querySelector('.apply-btn');
    applyBtn.addEventListener('click', (e) => {
        e.stopPropagation();
        const jobData = JSON.parse(applyBtn.getAttribute('data-job'));

        document.getElementById('modalJobTitle').textContent = `申请职位 - ${jobData.title}`;
        document.getElementById('detailTitle').textContent = jobData.title;
        document.getElementById('detailCompany').textContent = jobData.company;
        document.getElementById('detailLocation').textContent = jobData.location;
        document.getElementById('detailSalary').textContent = jobData.salary;
        document.getElementById('detailExperience').textContent = jobData.experience;
        document.getElementById('detailEducation').textContent = jobData.education;
        document.getElementById('detailIndustry').textContent = jobData.industry;
        document.getElementById('detailTags').innerHTML = jobData.tags.map(t => `<span class="job-tag" style="margin-right:5px;">${t}</span>`).join('');

        const websiteInfo = document.getElementById('websiteInfo');
        websiteInfo.innerHTML = `
            <p><strong>平台名称：</strong> 红途职引招聘平台</p>
            <p><strong>职位编号：</strong> ${jobData.id || '暂无'}</p>
            <p><strong>申请截止：</strong> ${new Date().getFullYear() + 1}-${(new Date().getMonth() + 1).toString().padStart(2, '0')}-${new Date().getDate().toString().padStart(2, '0')}</p>
            <p><strong>审核周期：</strong> 3-5个工作日</p>
            ${jobData.url ? `<p><strong>原始链接：</strong> <a href="${jobData.url}" target="_blank" style="color:#b91c1c;word-break:break-all;">${jobData.url}</a></p>` : ''}
            <p><strong>温馨提示：</strong> 请确保您的简历信息完整准确，平台将严格保护您的个人隐私。</p>
        `;

        // 每次打开弹窗时动态绑定"确认申请"按钮的跳转目标
        const modal = document.getElementById('jobModal');
        modal.dataset.applyUrl = jobData.url || '';
        const confirmBtnDynamic = modal.querySelector('.modal-confirm');
        if (confirmBtnDynamic) {
            // 移除旧的事件，用 onclick 直接覆盖（简单可靠）
            confirmBtnDynamic.onclick = function () {
                if (jobData.url) {
                    window.open(jobData.url, '_blank');
                } else {
                    alert('申请已提交！我们会尽快处理您的申请。');
                }
                modal.classList.remove('active');
                modal.style.display = 'none';
            };
        }
        modal.classList.add('active');
        modal.style.display = 'flex';
    });

    return div;
}

function initModalClose() {
    const modal = document.getElementById('jobModal');
    if (!modal) return;

    const closeBtn = document.getElementById('closeModal');
    const cancelBtn = document.getElementById('modalCancel');
    const confirmBtn = document.querySelector('.modal-confirm');

    function closeModal() {
        modal.classList.remove('active');
        modal.style.display = 'none';
    }

    if (closeBtn) {
        closeBtn.addEventListener('click', closeModal);
    }

    if (cancelBtn) {
        cancelBtn.addEventListener('click', closeModal);
    }

    if (confirmBtn) {
        // 兜底：如果 onclick 未被 createJobCard 覆盖，则直接关闭弹窗
        confirmBtn.addEventListener('click', function () {
            const applyUrl = modal.dataset.applyUrl;
            if (applyUrl && !this.onclick) {
                window.open(applyUrl, '_blank');
            }
            closeModal();
        });
    }

    modal.addEventListener('click', function (e) {
        if (e.target === modal) {
            closeModal();
        }
    });

    document.addEventListener('keydown', function (e) {
        if (e.key === 'Escape' && modal.classList.contains('active')) {
            closeModal();
        }
    });
}

// ========== DOMContentLoaded 主入口 ==========
document.addEventListener('DOMContentLoaded', async function () {
    // 【优先绑定】导航事件和初始状态 — 放在最前面，确保不受后续 await fetch 阻塞
    if (navHomeLink) navHomeLink.addEventListener("click", showHome);
    if (navRedLink) navRedLink.addEventListener("click", showRedGuide);
    if (goHomeLogo) goHomeLogo.addEventListener("click", showHome);
    // 绑定 PDF 弹窗关闭事件
    const pdfModal = document.getElementById("pdfViewerModal");
    if (pdfModal) {
        pdfModal.addEventListener("click", function(e) {
            if (e.target === pdfModal) closePdfModal();
        });
    }
    const pdfCloseBtn = document.getElementById("closePdfModal");
    if (pdfCloseBtn) pdfCloseBtn.addEventListener("click", closePdfModal);
    // 设置初始显示状态：首页显示，红色指引隐藏
    const redEl = getRedModule();
    if (redEl) redEl.style.display = "none";
    setActiveNav(false);
    destroyAutoPlay();

    // 以下为数据加载（异步，不阻塞交互）
    try {
        await fetchRedGuideData();
    } catch (e) {
        console.log("红色指引数据加载异常：", e);
    }

    // 初始化首页演示数据（使用后端获取的数据或默认数据）
    initHomeDemoData();

    // 初始化红色指引模块（预渲染，但隐藏）
    initRedGuideModule();

    // 初始化弹窗关闭逻辑
    initModalClose();

    // 从后端获取真实职位数据（Final_Version原有逻辑，完全不变）
    try {
        const jobRes = await fetch("http://localhost:8080/api/jobs");
        const jobResponse = await jobRes.json();
        jobsData = jobResponse.data.map(item => ({
            id: item.id,
            title: item.col_001 || item.post,
            company: item.col_006 || item.company,
            location: item.col_005 || item.region,
            experience: (item.col_007 == '-1' || item.col_007 == -1) ? '人数不限' : (item.col_007 || item.number || '-'),
            education: item.col_002 || item.qualification,
            salary: `${item.col_003 || item.salary_low || ''}-${item.col_004 || item.salary_high || ''}`,
            industry: item.col_008 || item.platform,
            tags: [item.col_009, item.col_010].filter(tag => tag),
            url: item.col_010 || item.url || ''
        }));

        renderIndustryOptions();
        updateRegions('all');
        renderJobs('all');

    } catch (err) {
        console.error("后端连接失败：", err);
        alert("无法连接后端服务器" + err.message);
    }

    document.querySelector('.search-btn').addEventListener('click', function () {
        const keyword = document.querySelector('.search-input').value.trim();
        if (!keyword) return alert("请输入关键词！");
        alert(`执行搜索：${keyword}`);
    });
});


//let industryRegions = {
//    "all": [
//        "全国",
//        "北京", "天津", "河北", "山西", "内蒙古",
//        "辽宁", "吉林", "黑龙江",
//        "上海", "江苏", "浙江", "安徽", "福建", "江西", "山东",
//        "河南", "湖北", "湖南", "广东", "广西", "海南",
//        "重庆", "四川", "贵州", "云南", "西藏",
//        "陕西", "甘肃", "青海", "宁夏", "新疆",
//        "香港", "澳门"
//    ]
//};
//let jobsData = [];

//document.addEventListener('DOMContentLoaded', async function () {
//    try {
//        // 从 C++ 后端获取数据
//        // const industryRes = await fetch("http://localhost:8080/api/industries");
//        const jobRes = await fetch("http://localhost:8080/api/jobs");

//        // industryRegions = await industryRes.json();

//        const jobResponse = await jobRes.json();
//        // 将新格式的数据转换为jobsData
//        jobsData = jobResponse.data.map(item => ({
//            id: item.id,
//            title: item.col_001,
//            company: item.col_006,
//            location: item.col_005,
//            experience: item.col_007,
//            education: item.col_002,
//            salary: `${item.col_003}-${item.col_004}`,
//            industry: item.col_008,  // 使用col_008作为行业
//            tags: [item.col_009, item.col_010].filter(tag => tag)// col_009和col_010作为tags
//        }));

//        renderIndustryOptions();
//        updateRegions('all');
//        renderJobs('all');

//    } catch (err) {
//        console.error("后端连接失败：", err);
//        alert("无法连接后端服务器" + err.message);
//    }

//    document.querySelector('.search-btn').addEventListener('click', function () {
//        const keyword = document.querySelector('.search-input').value.trim();
//        if (!keyword) return alert("请输入关键词！");
//        alert(`执行搜索：${keyword}`);
//    });
//});

//function renderIndustryOptions() {
//    const industryContainer = document.getElementById('industry-options');
//    industryContainer.innerHTML = '';

//    Object.keys(industryRegions).forEach((key, index) => {
//        const div = document.createElement('div');
//        div.className = `filter-option ${index === 0 ? 'active' : ''}`;
//        div.setAttribute('data-industry', key);
//        div.textContent = key === 'all' ? '全部' : key;
//        div.addEventListener('click', () => {
//            document.querySelectorAll('#industry-options .filter-option')
//                .forEach(opt => opt.classList.remove('active'));
//            div.classList.add('active');
//            updateRegions(key);
//            renderJobs(key);
//        });
//        industryContainer.appendChild(div);
//    });
//}

//function updateRegions(industry) {
//    const regionsContainer = document.getElementById('region-options');
//    regionsContainer.innerHTML = '';

//    (industryRegions[industry] || []).forEach((region, i) => {
//        const regionOption = document.createElement('div');
//        regionOption.className = `filter-option region-option ${i === 0 ? 'active' : ''}`;
//        regionOption.textContent = region;
//        regionOption.addEventListener('click', () => {
//            document.querySelectorAll('#region-options .filter-option')
//                .forEach(opt => opt.classList.remove('active'));
//            regionOption.classList.add('active');
//            filterJobsByRegion(region);
//        });
//        regionsContainer.appendChild(regionOption);
//    });
//}

//function renderJobs(industry) {
//    const jobsGrid = document.getElementById('jobs-container');
//    jobsGrid.innerHTML = '';
//    const filtered = industry === 'all' ? jobsData : jobsData.filter(j => j.industry === industry);
//    filtered.forEach(job => jobsGrid.appendChild(createJobCard(job)));
//}

//function filterJobsByRegion(region) {
//    const industry = document.querySelector('#industry-options .active').getAttribute('data-industry');
//    const jobsGrid = document.getElementById('jobs-container');
//    jobsGrid.innerHTML = '';
//    let filtered = industry === 'all' ? jobsData : jobsData.filter(j => j.industry === industry);
//    if (region !== '全国') filtered = filtered.filter(j => j.location === region);
//    filtered.forEach(job => jobsGrid.appendChild(createJobCard(job)));
//}

//function createJobCard(job) {
//    const div = document.createElement('div');
//    div.className = 'job-card';
//    div.innerHTML = `
//    <div class="job-header">
//      <h3 class="job-title">${job.title}</h3>
//      <div class="company">
//        <div class="company-logo">${job.company.substring(0, 2)}</div>
//        <div class="company-name">${job.company}</div>
//      </div>
//    </div>
//    <div class="job-details">
//      <div class="job-info">
//        <div class="job-info-item"><i class="fas fa-map-marker-alt"></i> <span>${job.location}</span></div>
//        <div class="job-info-item"><i class="fas fa-briefcase"></i> <span>${job.experience}</span></div>
//        <div class="job-info-item"><i class="fas fa-graduation-cap"></i> <span>${job.education}</span></div>
//      </div>
//      <div class="job-tags">
//        ${job.tags.map(t => `<span class="job-tag">${t}</span>`).join('')}
//      </div>
//      <div class="job-actions">
//        <div class="job-salary">${job.salary}</div>
//        <button class="btn btn-primary apply-btn" style="padding:5px 15px;font-size:14px;" data-job='${JSON.stringify(job).replace(/'/g, "&#39;")}'>申请</button>
//      </div>
//    </div>`;

//    // 为申请按钮添加点击事件
//    const applyBtn = div.querySelector('.apply-btn');
//    applyBtn.addEventListener('click', (e) => {
//        e.stopPropagation();
//        const jobData = JSON.parse(applyBtn.getAttribute('data-job'));

//        // 填充弹窗内容
//        document.getElementById('modalJobTitle').textContent = `申请职位 - ${jobData.title}`;
//        document.getElementById('detailTitle').textContent = jobData.title;
//        document.getElementById('detailCompany').textContent = jobData.company;
//        document.getElementById('detailLocation').textContent = jobData.location;
//        document.getElementById('detailSalary').textContent = jobData.salary;
//        document.getElementById('detailExperience').textContent = jobData.experience;
//        document.getElementById('detailEducation').textContent = jobData.education;
//        document.getElementById('detailIndustry').textContent = jobData.industry;
//        document.getElementById('detailTags').innerHTML = jobData.tags.map(t => `<span class="job-tag" style="margin-right:5px;">${t}</span>`).join('');

//        // 模拟平台信息
//        const websiteInfo = document.getElementById('websiteInfo');
//        websiteInfo.innerHTML = `
//            <p><strong>平台名称：</strong> 红途职引招聘平台</p>
//            <p><strong>职位编号：</strong> ${jobData.id || '暂无'}</p>
//            <p><strong>申请截止：</strong> ${new Date().getFullYear() + 1}-${(new Date().getMonth() + 1).toString().padStart(2, '0')}-${new Date().getDate().toString().padStart(2, '0')}</p>
//            <p><strong>审核周期：</strong> 3-5个工作日</p>
//            <p><strong>温馨提示：</strong> 请确保您的简历信息完整准确，平台将严格保护您的个人隐私。</p>
//        `;

//        // 显示弹窗
//        const modal = document.getElementById('jobModal');
//        modal.classList.add('active');
//        modal.style.display = 'flex';
//    });

//    return div;
//}

//(function initModalClose() {
//    const modal = document.getElementById('jobModal');
//    if (!modal) return;

//    const closeBtn = document.getElementById('closeModal');
//    const cancelBtn = document.getElementById('modalCancel');
//    const confirmBtn = document.querySelector('.modal-confirm');

//    function closeModal() {
//        modal.classList.remove('active');
//        modal.style.display = 'none';
//    }

//    if (closeBtn) {
//        closeBtn.addEventListener('click', closeModal);
//    }

//    if (cancelBtn) {
//        cancelBtn.addEventListener('click', closeModal);
//    }

//    if (confirmBtn) {
//        confirmBtn.addEventListener('click', function () {
//            alert('申请已提交！我们会尽快处理您的申请。');
//            closeModal();
//        });
//    }

//    modal.addEventListener('click', function (e) {
//        if (e.target === modal) {
//            closeModal();
//        }
//    });

//    document.addEventListener('keydown', function (e) {
//        if (e.key === 'Escape' && modal.classList.contains('active')) {
//            closeModal();
//        }
//    });

//})();