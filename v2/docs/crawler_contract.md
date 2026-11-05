# 爬虫组数据对接规范 v2.0

## 一、数据库信息

| 项目 | 值 |
|------|-----|
| 数据库类型 | MySQL 8.0 |
| 服务器地址 | 127.0.0.1:3306 |
| 数据库名 | htzy |
| 账号 | htzy_crawler |
| 密码 | 见 app.conf / 部署管理员 |

## 二、岗位表 (jobs)

爬虫需要往 `jobs` 表写入数据。字段定义如下：

| 字段 | 类型 | 必填 | 说明 |
|------|------|------|------|
| job_title | VARCHAR(200) | ✅ | 岗位名称 |
| company_name | VARCHAR(200) | ✅ | 公司名称 |
| location | VARCHAR(100) | ❌ | 工作地点（详细地址） |
| city | VARCHAR(50) | ❌ | 城市（用于匹配筛选） |
| salary_min | INT | ❌ | 最低薪资（元/月） |
| salary_max | INT | ❌ | 最高薪资（元/月） |
| salary_text | VARCHAR(100) | ❌ | 薪资描述文本，如 "8K-15K" |
| education | VARCHAR(50) | ❌ | 学历要求 |
| experience | VARCHAR(50) | ❌ | 经验要求 |
| industry | VARCHAR(100) | ❌ | 行业分类 |
| job_tags | JSON | ❌ | 标签数组，如 ["Python","Django"] |
| job_desc | MEDIUMTEXT | ❌ | 岗位描述 |
| source_url | VARCHAR(500) | ✅ | 来源链接（唯一键，用于去重） |
| source_platform | VARCHAR(100) | ❌ | 来源平台名，默认 "CollegeEmploymentService" |
| publish_date | DATE | ❌ | 发布日期 |
| **contact_email** | **VARCHAR(200)** | **❌** | **公司招聘邮箱（重要！用于自动投递功能）** |

### ⚠️ contact_email 字段说明

**这是自动投递功能的核心字段。** 当 `contact_email` 为空或 NULL 时，系统视该公司为**无法自动投递状态**，自动投递算法会跳过这些岗位。

- 如果爬虫能获取到公司招聘邮箱，请务必填写
- 如果获取不到，该字段留空即可（不影响岗位展示，仅影响自动投递）
- 建议在招聘网站的职位详情页、公司主页、或 HR 邮箱列表中寻找

### 数据写入示例 (SQL)

```sql
INSERT INTO jobs (job_title, company_name, location, city, salary_min, salary_max,
    salary_text, education, experience, industry, job_desc, source_url,
    source_platform, publish_date, contact_email)
VALUES (
    'Python后端开发工程师',
    '长春某某科技有限公司',
    '吉林省长春市朝阳区',
    '长春',
    8000, 15000,
    '8K-15K',
    '本科',
    '1-3年',
    '互联网/软件',
    '负责公司后端系统开发...',
    'https://example.com/job/12345',
    'CollegeEmploymentService',
    '2026-09-19',
    'hr@example.com'
);
```

### 去重策略

使用 `source_url` 作为唯一键去重。写入前检查：

```sql
SELECT id FROM jobs WHERE source_url = '目标URL';
```

如果已存在则跳过或 UPDATE。

## 三、红色指引表 (red_stories)

爬虫可选写入红色故事数据：

| 字段 | 类型 | 必填 | 说明 |
|------|------|------|------|
| title | VARCHAR(200) | ✅ | 标题 |
| summary | VARCHAR(500) | ✅ | 摘要 |
| content | TEXT | ✅ | 正文内容 |
| category | VARCHAR(50) | ❌ | 分类：computer_pioneer / revolutionary / model_worker |
| source_url | VARCHAR(500) | ❌ | 来源链接 |
| source_name | VARCHAR(100) | ❌ | 来源名称 |

## 四、注意事项

1. 所有文本字段使用 UTF-8 编码
2. 插入前检查 source_url 是否已存在，避免重复
3. job_desc 字段为 MEDIUMTEXT，可存储较长内容
4. contact_email 建议格式：user@example.com 或 user@example.com
5. 建议每批次插入不超过 100 条，使用事务
