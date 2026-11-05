

(function () {
    // ---------- 1. 首页演示数据填充 (保持原有展示) ----------
    const industries = ["互联网/IT", "金融", "教育培训", "医疗健康", "制造业", "红色文旅"];
    const regions = ["北京", "上海", "广州", "深圳", "杭州", "成都"];
    const jobsData = [
        { title: "前端开发工程师", company: "红色基因科技", location: "北京", salary: "20-35K", industry: "互联网" },
        { title: "红色文旅策划", company: "星火文化传媒", location: "延安", salary: "10-15K", industry: "文旅" },
        { title: "基层党务助理", company: "乡村振兴促进会", location: "江西井冈山", salary: "6-9K", industry: "公共事业" },
        { title: "Java后端工程师", company: "浪潮云", location: "济南", salary: "18-28K", industry: "互联网" },
        { title: "红色教育讲师", company: "红源干部学院", location: "遵义", salary: "12-18K", industry: "教育" },
        { title: "数据分析师", company: "数智红途", location: "杭州", salary: "16-25K", industry: "互联网" }
    ];

    const indusContainer = document.getElementById("demoIndustries");
    if (indusContainer) {
        industries.forEach(ind => { let span = document.createElement("span"); span.className = "filter-option"; span.innerText = ind; indusContainer.appendChild(span); });
    }
    const regionContainer = document.getElementById("demoRegions");
    if (regionContainer) {
        regions.forEach(reg => { let span = document.createElement("span"); span.className = "filter-option"; span.innerText = reg; regionContainer.appendChild(span); });
    }
    const jobsGrid = document.getElementById("jobsDemoGrid");
    if (jobsGrid) {
        jobsData.forEach(job => {
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

    // ---------- 2. 红色指引核心数据 (轮播图 + 故事卡片) 符合红色主题 ----------
    const carouselItems = [
        { image: "https://picsum.photos/id/1002/1200/450", title: "红色精神 科技报国", description: "从“两弹一星”到国产芯片，计算机事业在红色指引下蓬勃发展" },
        { image: "https://picsum.photos/id/1015/1200/450", title: "乡村振兴 数字赋能", description: "数字化基层就业，为乡村振兴插上科技翅膀" },
        { image: "https://picsum.photos/id/1039/1200/450", title: "红色教育基地 · 职业成长", description: "联合红色遗址与高校，打造计算机科技人才培养计划" }
    ];

    const storyCards = [
        { icon: "fa-microchip", title: "红色计算机记忆", content: "从1956年计算所诞生到国产操作系统突破，一代代科研人员赓续红色血脉，突破技术封锁。" },
        { icon: "fa-network-wired", title: "网信事业初心", content: "坚持自主创新，构建网络安全防线，红色指引下数字中国战略稳步推进。" },
        { icon: "fa-chalkboard-user", title: "基层数字化帮扶", content: "开展乡村电商、智慧农业培训，以信息技术助力老区振兴。" },
        { icon: "fa-hand-holding-heart", title: "退役军人IT扶持", content: "联合龙头企业提供编程、运维等定向培训，护航退役军人职业转型。" },
        { icon: "fa-people-arrows", title: "红色企业对接", content: "汇聚信创、智慧党建等领域优质岗位，搭建精准就业桥梁。" },
        { icon: "fa-chart-line", title: "红色职业规划", content: "传承艰苦奋斗精神，为青年提供长期技术成长路线与价值观引导。" }
    ];

    // 轮播图状态变量
    let currentSlideIdx = 0;
    let slideCounts = carouselItems.length;
    let autoTimer = null;

    // DOM 元素
    const slidesContainer = document.getElementById("redCarouselSlides");
    const indicatorsContainer = document.getElementById("redIndicators");
    const storiesContainer = document.getElementById("redStoriesGrid");
    let prevBtn = document.getElementById("redPrevBtn");
    let nextBtn = document.getElementById("redNextBtn");
    const carouselWrapper = document.querySelector("#redGuideModule .carousel-container");

    // 辅助函数：转义HTML
    function escapeHtml(str) {
        if (!str) return "";
        return str.replace(/[&<>]/g, function (m) {
            if (m === '&') return '&amp;';
            if (m === '<') return '&lt;';
            if (m === '>') return '&gt;';
            return m;
        });
    }

    // 渲染轮播图
    function renderCarousel() {
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

    // 渲染故事卡片
    function renderStories() {
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

    // 更新轮播位置及指示器
    function updateCarouselPosition() {
        if (!slidesContainer) return;
        slidesContainer.style.transform = `translateX(-${currentSlideIdx * 100}%)`;
        const indicators = document.querySelectorAll("#redIndicators .indicator");
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

    // 重新绑定事件（避免重复监听，用于显示时刷新）
    function rebindCarouselEvents() {
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
        const freshPrev = document.getElementById("redPrevBtn");
        const freshNext = document.getElementById("redNextBtn");
        if (freshPrev) freshPrev.addEventListener("click", () => { goToSlide(currentSlideIdx - 1); });
        if (freshNext) freshNext.addEventListener("click", () => { goToSlide(currentSlideIdx + 1); });

        // 重新绑定指示器事件
        const indicators = document.querySelectorAll("#redIndicators .indicator");
        indicators.forEach((ind, idx) => {
            ind.removeEventListener("click", () => goToSlide(idx));
            ind.addEventListener("click", () => goToSlide(idx));
        });
    }

    // 完全初始化红色模块（重置并渲染）
    function initRedGuideModule() {
        renderCarousel();
        renderStories();
        rebindCarouselEvents();
        if (carouselWrapper) {
            // 先移除原有监听避免重复，再添加
            carouselWrapper.removeEventListener("mouseenter", stopAutoPlay);
            carouselWrapper.removeEventListener("mouseleave", startAutoPlay);
            carouselWrapper.addEventListener("mouseenter", stopAutoPlay);
            carouselWrapper.addEventListener("mouseleave", startAutoPlay);
        }
        startAutoPlay();
    }

    function destroyAutoPlay() {
        if (autoTimer) clearInterval(autoTimer);
        autoTimer = null;
    }

    // ---------- 3. 页面切换核心逻辑 (导航栏不变，完全切换模块) ----------
    const homeContainer = document.getElementById("homePageContainer");
    const redModule = document.getElementById("redGuideModule");
    const navHomeLink = document.getElementById("navHome");
    const navRedLink = document.getElementById("navRedGuide");
    const goHomeLogo = document.getElementById("goHomeBtn");

    // 高亮当前导航项
    function setActiveNav(isRedActive) {
        if (isRedActive) {
            navRedLink.classList.add("red-active");
            navHomeLink.classList.remove("red-active");
            navRedLink.style.color = "#dc2626";
            if (navHomeLink) navHomeLink.style.color = "";
        } else {
            navHomeLink.classList.add("red-active");
            navRedLink.classList.remove("red-active");
            navHomeLink.style.color = "#dc2626";
            if (navRedLink) navRedLink.style.color = "";
        }
    }

    function showHome() {
        if (homeContainer) homeContainer.style.display = "block";
        if (redModule) redModule.style.display = "none";
        setActiveNav(false);
        destroyAutoPlay();   // 离开红色模块停止轮播
    }

    function showRedGuide() {
        if (homeContainer) homeContainer.style.display = "none";
        if (redModule) redModule.style.display = "block";
        setActiveNav(true);
        // 每次显示红色指引重新初始化，确保轮播状态最新，且自动播放正常
        // 重置索引到0并重新渲染所有内容，保证数据与视觉同步
        currentSlideIdx = 0;
        // 重新渲染轮播图和卡片，并重新绑定事件
        renderCarousel();
        renderStories();
        rebindCarouselEvents();
        // 重新挂载容器事件
        if (carouselWrapper) {
            carouselWrapper.removeEventListener("mouseenter", stopAutoPlay);
            carouselWrapper.removeEventListener("mouseleave", startAutoPlay);
            carouselWrapper.addEventListener("mouseenter", stopAutoPlay);
            carouselWrapper.addEventListener("mouseleave", startAutoPlay);
        }
        startAutoPlay();
    }

    // 绑定导航事件
    if (navHomeLink) navHomeLink.addEventListener("click", showHome);
    if (navRedLink) navRedLink.addEventListener("click", showRedGuide);
    if (goHomeLogo) goHomeLogo.addEventListener("click", showHome);

    // 初始化: 默认显示首页，红色模块隐藏，但预加载数据
    redModule.style.display = "none";
    homeContainer.style.display = "block";
    setActiveNav(false);
    // 预渲染红色模块内容（数据填充，但隐藏）
    renderCarousel();
    renderStories();
    rebindCarouselEvents();
    if (carouselWrapper) {
        carouselWrapper.addEventListener("mouseenter", stopAutoPlay);
        carouselWrapper.addEventListener("mouseleave", startAutoPlay);
    }
    // 由于模块隐藏，自动播放先停止，等显示时再启动 (避免后台运行)
    destroyAutoPlay();
})();