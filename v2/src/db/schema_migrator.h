// SchemaMigrator - Auto create DB/tables + crawler user (idempotent)
#pragma once

#include <string>
#include <random>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "../common/logger.h"
#include "../common/config.h"
#include "../common/jwt.h"
#include "database.h"

class SchemaMigrator {
public:
    static bool migrate(const AppConfig& cfg) {
        LOG_INFO("Migrate", "=== DB Init Start ===");

        Database& db = Database::instance();
        if (!db.connectWithoutDb(cfg.database.host, cfg.database.port,
                                 cfg.database.user, cfg.database.password)) {
            LOG_ERROR("Migrate", "Cannot connect to MySQL at " + cfg.database.host + ":" + std::to_string(cfg.database.port));
            LOG_ERROR("Migrate", "Check: 1) MySQL service is running  2) root password is correct  3) mysqlcppconn DLL is in output dir");
            return false;
        }

        if (!db.createDatabase(cfg.database.dbname)) {
            LOG_ERROR("Migrate", "Failed to create database");
            return false;
        }
        LOG_INFO("Migrate", "Database " + cfg.database.dbname + " ready");

        if (!db.useDatabase(cfg.database.dbname)) {
            LOG_ERROR("Migrate", "Failed to switch database");
            return false;
        }

        if (!createAllTables(db)) {
            LOG_ERROR("Migrate", "Failed to create tables");
            return false;
        }

        std::string crawler_password;
        if (!createCrawlerUser(db, crawler_password)) {
            LOG_ERROR("Migrate", "Failed to create crawler user");
            return false;
        }

        if (!writeCrawlerConfig(cfg, crawler_password)) {
            LOG_ERROR("Migrate", "Failed to write crawler config");
            return false;
        }

        db.useDatabase(cfg.database.dbname);
        LOG_INFO("Migrate", "=== DB Init Done ===");
        return true;
    }

private:
    static bool createAllTables(Database& db) {
        // Students roster table (uploaded by admin, used for registration verification)
        db.execute(R"(
            CREATE TABLE IF NOT EXISTS students (
                id INT AUTO_INCREMENT PRIMARY KEY,
                student_id VARCHAR(20) NOT NULL UNIQUE COMMENT 'Student number',
                real_name VARCHAR(50) NOT NULL,
                class_name VARCHAR(50) NULL COMMENT 'Class name, e.g. CS2401',
                department VARCHAR(100) NULL COMMENT 'Department/major',
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                INDEX idx_student_id (student_id)
            ) ENGINE=InnoDB
        )");
        LOG_INFO("Migrate", "students table ready");

        // Users table (students + admins)
        db.execute(R"(
            CREATE TABLE IF NOT EXISTS users (
                id INT AUTO_INCREMENT PRIMARY KEY,
                username VARCHAR(50) NOT NULL UNIQUE,
                password_hash VARCHAR(128) NOT NULL,
                salt VARCHAR(32) NOT NULL,
                email VARCHAR(100) NOT NULL UNIQUE,
                phone VARCHAR(20) NULL,
                avatar VARCHAR(255) NULL,
                bio TEXT NULL,
                role ENUM('student','admin','super_admin') DEFAULT 'student',
                student_id VARCHAR(20) NULL UNIQUE COMMENT 'Linked student number',
                real_name VARCHAR(50) NULL,
                class_name VARCHAR(50) NULL,
                is_verified TINYINT DEFAULT 0 COMMENT '1=student verified by roster',
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                last_login DATETIME NULL,
                interview_count INT DEFAULT 0,
                INDEX idx_role (role),
                INDEX idx_verified (is_verified)
            ) ENGINE=InnoDB
        )");
        LOG_INFO("Migrate", "users table ready");

        db.execute(R"(
            CREATE TABLE IF NOT EXISTS resumes (
                id INT AUTO_INCREMENT PRIMARY KEY,
                user_id INT NOT NULL,
                file_path VARCHAR(255) NOT NULL,
                content MEDIUMTEXT NULL,
                upload_time DATETIME DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
                INDEX idx_user (user_id)
            ) ENGINE=InnoDB
        )");
        LOG_INFO("Migrate", "resumes table ready");

        db.execute(R"(
            CREATE TABLE IF NOT EXISTS interviews (
                id INT AUTO_INCREMENT PRIMARY KEY,
                user_id INT NOT NULL,
                job_position VARCHAR(100) NOT NULL,
                status ENUM('pending','in_progress','completed','aborted') DEFAULT 'pending',
                start_time DATETIME NULL,
                end_time DATETIME NULL,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
                INDEX idx_user_status (user_id, status)
            ) ENGINE=InnoDB
        )");
        LOG_INFO("Migrate", "interviews table ready");

        db.execute(R"(
            CREATE TABLE IF NOT EXISTS interview_questions (
                id INT AUTO_INCREMENT PRIMARY KEY,
                interview_id INT NOT NULL,
                question_text TEXT NOT NULL,
                question_type VARCHAR(50) NOT NULL,
                difficulty TINYINT NOT NULL,
                order_number INT NOT NULL,
                FOREIGN KEY (interview_id) REFERENCES interviews(id) ON DELETE CASCADE,
                INDEX idx_interview (interview_id, order_number)
            ) ENGINE=InnoDB
        )");
        LOG_INFO("Migrate", "interview_questions table ready");

        db.execute(R"(
            CREATE TABLE IF NOT EXISTS interview_answers (
                id INT AUTO_INCREMENT PRIMARY KEY,
                interview_id INT NOT NULL,
                question_id INT NOT NULL,
                answer_text MEDIUMTEXT NULL,
                start_time DATETIME NULL,
                end_time DATETIME NULL,
                FOREIGN KEY (interview_id) REFERENCES interviews(id) ON DELETE CASCADE,
                FOREIGN KEY (question_id) REFERENCES interview_questions(id) ON DELETE CASCADE,
                UNIQUE KEY uq_question (question_id)
            ) ENGINE=InnoDB
        )");
        LOG_INFO("Migrate", "interview_answers table ready");

        db.execute(R"(
            CREATE TABLE IF NOT EXISTS interview_results (
                id INT AUTO_INCREMENT PRIMARY KEY,
                interview_id INT NOT NULL,
                professional_score DECIMAL(5,2) NOT NULL DEFAULT 0,
                language_score DECIMAL(5,2) NOT NULL DEFAULT 0,
                expression_score DECIMAL(5,2) NOT NULL DEFAULT 0,
                overall_score DECIMAL(5,2) NOT NULL DEFAULT 0,
                report MEDIUMTEXT NULL,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (interview_id) REFERENCES interviews(id) ON DELETE CASCADE,
                UNIQUE KEY uq_interview (interview_id)
            ) ENGINE=InnoDB
        )");
        LOG_INFO("Migrate", "interview_results table ready");

        db.execute(R"(
            CREATE TABLE IF NOT EXISTS facial_analysis (
                id INT AUTO_INCREMENT PRIMARY KEY,
                interview_id INT NOT NULL,
                frame_number INT NOT NULL,
                stress_level DECIMAL(5,2) NOT NULL DEFAULT 0,
                dressing_neat DECIMAL(5,2) NOT NULL DEFAULT 0,
                eye_contact DECIMAL(5,2) NOT NULL DEFAULT 0,
                facial_express DECIMAL(5,2) NOT NULL DEFAULT 0,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (interview_id) REFERENCES interviews(id) ON DELETE CASCADE,
                INDEX idx_interview (interview_id)
            ) ENGINE=InnoDB
        )");
        LOG_INFO("Migrate", "facial_analysis table ready");

        db.execute(R"(
            CREATE TABLE IF NOT EXISTS voice_analysis (
                id INT AUTO_INCREMENT PRIMARY KEY,
                interview_id INT NOT NULL,
                mandarin DECIMAL(5,2) NOT NULL DEFAULT 0,
                voice_stability DECIMAL(5,2) NOT NULL DEFAULT 0,
                clarity DECIMAL(5,2) NOT NULL DEFAULT 0,
                organization DECIMAL(5,2) NOT NULL DEFAULT 0,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (interview_id) REFERENCES interviews(id) ON DELETE CASCADE,
                INDEX idx_interview (interview_id)
            ) ENGINE=InnoDB
        )");
        LOG_INFO("Migrate", "voice_analysis table ready");

        db.execute(R"(
            CREATE TABLE IF NOT EXISTS jobs (
                id BIGINT AUTO_INCREMENT PRIMARY KEY,
                job_title VARCHAR(200) NOT NULL,
                company_name VARCHAR(200) NOT NULL,
                location VARCHAR(100) NULL,
                province VARCHAR(50) NULL,
                city VARCHAR(50) NULL,
                salary_min INT NULL,
                salary_max INT NULL,
                salary_text VARCHAR(100) NULL,
                education VARCHAR(50) NULL,
                experience VARCHAR(50) NULL,
                industry VARCHAR(100) NULL,
                job_tags JSON NULL,
                job_desc MEDIUMTEXT NULL,
                source_url VARCHAR(500) NOT NULL,
                source_platform VARCHAR(100) DEFAULT 'CollegeEmploymentService',
                publish_date DATE NULL,
                crawl_time DATETIME DEFAULT CURRENT_TIMESTAMP,
                UNIQUE KEY uq_source_url (source_url(255)),
                INDEX idx_city (city),
                INDEX idx_industry (industry),
                INDEX idx_edu_exp (education, experience)
            ) ENGINE=InnoDB
        )");
        LOG_INFO("Migrate", "jobs table ready");

        db.execute(R"(
            CREATE TABLE IF NOT EXISTS red_stories (
                id BIGINT AUTO_INCREMENT PRIMARY KEY,
                title VARCHAR(200) NOT NULL,
                summary VARCHAR(500) NOT NULL,
                content TEXT NOT NULL,
                category VARCHAR(50) NOT NULL DEFAULT 'computer_pioneer',
                image_url VARCHAR(500) NULL,
                source_url VARCHAR(500) NULL,
                source_name VARCHAR(100) NULL,
                view_count INT DEFAULT 0,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                INDEX idx_category (category)
            ) ENGINE=InnoDB
        )");
        LOG_INFO("Migrate", "red_stories table ready");

        // Seed initial red stories data
        seedRedStories(db);

        // Seed super_admin account (username: superadmin, password: admin123)
        seedSuperAdmin(db);

        // Auto-migrate new columns/tables
        migrateJobsContactEmail(db);
        migrateDeliveryLogs(db);
        migrateDeliveryConfig(db);

        return true;
    }

    static void migrateJobsContactEmail(Database& db) {
        try {
            db.execute("ALTER TABLE jobs ADD COLUMN contact_email VARCHAR(200) NULL AFTER source_platform");
            LOG_INFO("Migrate", "Added contact_email to jobs table");
        } catch (...) { /* column already exists */ }
    }

    static void migrateDeliveryLogs(Database& db) {
        db.execute(R"(
            CREATE TABLE IF NOT EXISTS delivery_logs (
                id INT AUTO_INCREMENT PRIMARY KEY,
                user_id INT NOT NULL,
                resume_id INT NOT NULL,
                job_id BIGINT NOT NULL,
                company_email VARCHAR(200) NOT NULL,
                status ENUM('sent','failed','no_email') DEFAULT 'sent',
                sent_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
                INDEX idx_user_date (user_id, sent_at),
                INDEX idx_job (job_id)
            ) ENGINE=InnoDB
        )");
        LOG_INFO("Migrate", "delivery_logs table ready");
    }

    static void migrateDeliveryConfig(Database& db) {
        db.execute(R"(
            CREATE TABLE IF NOT EXISTS delivery_config (
                id INT AUTO_INCREMENT PRIMARY KEY,
                user_id INT NOT NULL UNIQUE,
                enabled TINYINT DEFAULT 0,
                resume_id INT DEFAULT 0,
                job_keywords VARCHAR(500) NULL,
                location_pref VARCHAR(100) NULL,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                updated_at DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
                FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
            ) ENGINE=InnoDB
        )");
        LOG_INFO("Migrate", "delivery_config table ready");
    }

    static void seedSuperAdmin(Database& db) {
        Rows exist = db.query("SELECT id FROM users WHERE role='super_admin' LIMIT 1");
        if (!exist.empty()) {
            LOG_INFO("Migrate", "Super admin already exists, skipping");
            return;
        }
        // Default super admin: username=superadmin, password=admin123
        // Compute SHA-256(salt + password) using jwt::sha256
        std::string raw_salt = "superadmin_seed_salt";
        std::string raw_pw = "admin123";
        std::string raw_hash_bytes = jwt::sha256(raw_salt + raw_pw);
        std::ostringstream oss;
        oss << std::hex << std::setfill('0');
        for (unsigned char c : raw_hash_bytes) oss << std::setw(2) << static_cast<int>(c);
        std::string hash = oss.str();

        std::string sql = "INSERT INTO users (username, password_hash, salt, email, role) VALUES ("
            "'superadmin','" + hash + "','" + raw_salt + "','admin@htzy.edu.cn','super_admin')";
        if (db.execute(sql)) {
            LOG_INFO("Migrate", "Super admin created: superadmin / admin123");
        } else {
            LOG_ERROR("Migrate", "Failed to create super admin");
        }
    }

    static void seedRedStories(Database& db) {
        // Check if already seeded with content
        Rows check = db.query("SELECT COUNT(*) AS cnt FROM red_stories");
        if (!check.empty() && std::stoi(check[0]["cnt"]) > 5) {
            LOG_INFO("Migrate", "Red stories already seeded (" + check[0]["cnt"] + " rows), skipping");
            return;
        }
        // Clear existing incomplete data and re-seed
        db.execute("DELETE FROM red_stories");

        auto esc = [](const std::string& s) -> std::string {
            std::string out;
            for (char c : s) {
                if (c == '\'') out += "\\'";
                else if (c == '\\') out += "\\\\";
                else out += c;
            }
            if (out.size() > 60000) out = out.substr(0, 60000);
            return out;
        };

        struct Story {
            std::string title, summary, content, category, source_url, source_name;
        };

        std::vector<Story> stories = {
            {
                "中国第一台计算机：103机的诞生",
                "1958年8月1日，中国第一台通用数字电子计算机103机研制成功，每秒运算30次，标志着中国计算机事业的起点。",
                "1958年8月1日，我国第一台电子计算机103机完成了四条指令的运行表演，每秒运算速度为30次，成为我国计算技术这门学科建立的标志。中国科学院原党组书记张劲夫风趣地给103机取了这个小名。"
                "新中国成立初期，国内的计算机学科还是一片荒地。我国著名数学家华罗庚在心中埋下一个梦想：要在中国研制出计算机。1951年，华罗庚组织闵乃大、夏培肃、王传英三位青年学者，组建起一支年轻的开垦队。"
                "1956年，周恩来总理亲自主持制定了十二年科技规划，计算技术被列为四项紧急措施之一。同年，中国科学院计算技术研究所筹备委员会成立，华罗庚担任筹备委员会主任。"
                "按照先集中后分散的原则，来自全国各地的20多人被划归计算所，陆续在全国范围内抽调了一批专业人才。大家来到计算所，为开创我国计算机事业同舟共济。"
                "1957年12月，苏联M-3图纸开始分批运抵北京，仿制工作立即展开。计算所决定仿制苏联大型计算机和M-3小型计算机作为突破口。经过全院集中攻关，1958年建军节，我国第一台计算机研制成功。",
                "computer_pioneer",
                "https://www.peopleapp.com/rmharticle/30013883119",
                "人民日报"
            },
            {
                "钱学森与中国计算机事业的创立",
                "钱学森力排众议，推动电子计算机列入国家四项紧急措施，为中国计算机事业奠定体制基础。",
                "钱学森虽非计算机专业研究者，但在计算机科学技术入列国家层面的科技规划过程中发挥了至关重要的作用。"
                "1956年1月，中共中央召开知识分子问题会议，周恩来总理做了大会报告，要求制定新中国第一个关于科学技术的远景规划。钱学森加入了由12名顶级科学家组成的综合组。"
                "在数次讨论中，电子计算机一度被认为发展前景不明朗。钱学森举出很多实例来说明发展电子计算机的重要性，比如水轮机设计的精确计算、计算机下象棋等，力主将计算技术列入国家科学规划。"
                "时任规划委秘书长的张劲夫后来回忆说：'在规划完成后又提出四项紧急措施，包括落实这些措施，学森同志也是立了大功的。'"
                "在钱学森等人的推动下，计算技术的建立被列入科学规划57项重要科学任务和四项紧急措施之一，为国家大力建设和发展计算机科学技术提供了体制保障。",
                "computer_pioneer",
                "https://kyff.cas.cn/mtlj/202507/t20250702_5075114.html",
                "中国科学院科学家精神网"
            },
            {
                "黄令仪：匍匐在地，擦干祖国身上的耻辱",
                "从氢弹到龙芯，82岁仍在一线，黄令仪用一生践行了'匍匐在地，擦净祖国身上的耻辱'。",
                "1989年，黄令仪受邀参加国际芯片展览会。偌大的会场，她跑遍了所有展台，没有找到一家中国企业，没有看到一块中国生产的芯片。她在日记中悲愤地写下八个字——'琳琅满目非国货，泪眼涟涟。'"
                "正是这份屈辱，让她更加坚定了自主研发中国芯片的决心。2001年，65岁的黄令仪接到中科院龙芯研发项目的邀请，二话没说放弃安逸的退休生活，一头又扎回了科研第一线。"
                "2002年8月10日，经过无数个日夜的拼搏攻坚，我国第一枚自主通用CPU龙芯1号研制成功。虽然对比同期国际顶尖芯片性能仍有差距，但它历史性终结了国产计算机没有自主CPU的局面。"
                "此后黄令仪继续带领团队攻坚克难。2018年龙芯3号研制成功，成为国之重器的心脏。复兴号高铁实现了百分之百的国产化，歼20等先进军事装备换上了中国自己的心脏，北斗卫星也装上了中国芯。"
                "她说过：'我此生最大的心愿，就是匍匐在地，擦净祖国身上的耻辱。'2023年4月20日，黄令仪与世长辞，走后骨灰撒入大海，一同被撒出的，还有一枚小小的芯片。",
                "chip_pioneer",
                "https://news.qq.com/rain/a/20260910A04MNV00",
                "腾讯新闻"
            },
            {
                "两弹一星精神：自力更生的科技报国之路",
                "没有进口计算机，就用手摇计算机和算盘。两弹一星的科学家们用最原始的工具创造了最伟大的奇迹。",
                "20世纪五六十年代，面对严峻的国际形势，以邓稼先、钱学森等为代表的无数顶尖科研专家、技术人员奔赴大西北的草原荒滩，隐姓埋名、扎根戈壁。"
                "原子弹理论设计研究没有图纸和模型，邓稼先就带领年轻人自行设计；没有进口的先进计算机，就用手摇计算机、计算尺甚至算盘来计算。封存在仓库中的几十麻袋计算草稿，是邓稼先率领的理论设计队伍艰苦攻关的记录。"
                "1964年10月16日15时，中国第一颗原子弹爆炸成功。从原子弹到氢弹，美国用了7年零3个月，苏联用了4年，我国仅用了2年零8个月。"
                "1970年4月24日，我国第一颗人造卫星发射成功，茫茫太空响起了东方红乐曲。我国成为继苏联、美国、法国、日本之后世界第5个发射卫星的国家。"
                "两弹一星精神：热爱祖国、无私奉献，自力更生、艰苦奋斗，大力协同、勇于攀登——这是中华民族的宝贵精神财富，激励着一代代科技工作者肩负起实现高水平科技自立自强的时代重任。",
                "two_bombs_one_satellite",
                "https://www.mod.gov.cn/gfbw/gfjy_index/4886742.html",
                "国防部"
            },
            {
                "夏培肃：被刻在国产芯片上的计算机女神",
                "1952年，夏培肃与华罗庚共同组建中国第一个计算机研究小组，开启了中国计算机事业的征程。",
                "夏培肃在英国已看到计算机发展的广阔前景。1952年秋天，在华罗庚的支持下，她与闵乃大、王传英组成我国第一个计算机研究小组。夏培肃在后来的回忆文章中称之为'改变命运的重要转折点'。"
                "当时国内没有一本叙述电子计算机原理的书，他们就从英文期刊中一点点查找，一个字一个字地抄录下来，制定了开发思路。"
                "1960年，夏培肃主持设计研制的107机调试成功，这是中国人自行研制的第一台通用计算机。该机连续稳定工作达20.5小时，远超仿制苏联的103机。"
                "夏培肃编写了《电子计算机原理》讲义，该书的精要内容一直沿用至今。她培养了一代又一代计算机专业人才，为中国计算机事业的开拓与发展奠定了坚实基础。",
                "computer_pioneer",
                "https://kyff.cas.cn/ff/dsff/202504/t20250416_5064339.html",
                "中国科学院科学家精神网"
            },
            {
                "龙芯24年：从无芯到纯血自主",
                "2002年龙芯1号诞生，24年后龙芯3C6000已媲美国际中高端CPU，中国芯片走出了一条最难的自主之路。",
                "1999年，科技部部长徐冠华说：'中国信息产业缺芯少魂。'中科院计算所拿出1000万家底上马CPU项目。项目负责人胡伟武立下军令状：完不成任务，提头来见。"
                "2002年8月10日凌晨，中科院计算所北楼105房间，安装了龙芯1号的计算机顺利启动。那是中国首款自主研发的通用CPU。"
                "最困难的时候，龙芯连工资都快发不出来了。2010年，北京市中关村管委会为了给龙芯拉来投资，开了26次协调会，最终国企注资带动民企跟投，龙芯拿到了2亿元股权投资。"
                "龙芯选择了一条最难的路——放弃合资捷径，构建纯血的龙架构，从指令系统到各种IP核全部自主研发，不依赖任何国外授权。"
                "2026年9月，龙芯3A6000桌面处理器主流行业出货量突破100万片。龙芯3C6000已进入量产倒计时，测试结果显示已媲美国际大厂中高端CPU产品。",
                "chip_pioneer",
                "https://www.toutiao.com/article/7681574280567587338/",
                "京报网"
            }
        };

        int count = 0;
        for (const auto& s : stories) {
            std::string sql = "INSERT INTO red_stories (title, summary, content, category, source_url, source_name) VALUES ('"
                + esc(s.title) + "','" + esc(s.summary) + "','" + esc(s.content) + "','"
                + esc(s.category) + "','" + esc(s.source_url) + "','" + esc(s.source_name) + "')";
            if (db.execute(sql)) count++;
        }
        LOG_INFO("Migrate", "Seeded " + std::to_string(count) + " red stories");
    }

    static bool createCrawlerUser(Database& db, std::string& out_password) {
        out_password = generateRandomPassword(16);
        std::string username = "htzy_crawler";

        std::string create_sql = "CREATE USER IF NOT EXISTS '" + username + "'@'localhost' IDENTIFIED BY '" + out_password + "'";
        if (!db.execute(create_sql)) {
            std::string alter_sql = "ALTER USER '" + username + "'@'localhost' IDENTIFIED BY '" + out_password + "'";
            db.execute(alter_sql);
        }

        std::string grant_sql =
            "GRANT SELECT, INSERT, UPDATE, DELETE ON htzy.jobs TO '" + username + "'@'localhost'";
        if (!db.execute(grant_sql)) {
            LOG_ERROR("Migrate", "GRANT failed");
            return false;
        }
        db.execute("FLUSH PRIVILEGES");

        LOG_INFO("Migrate", "Crawler user " + username + " created (jobs table only)");
        return true;
    }

    static bool writeCrawlerConfig(const AppConfig& cfg, const std::string& crawler_password) {
        std::string path = "config/crawler_db.conf";
        std::ofstream f(path);
        if (!f.is_open()) {
            LOG_ERROR("Migrate", "Cannot write: " + path);
            return false;
        }
        f << R"({"db_host":")" << cfg.database.host
          << R"(","db_port":)" << cfg.database.port
          << R"(,"db_name":")" << cfg.database.dbname
          << R"(","user":"htzy_crawler","password":")" << crawler_password
          << R"(","table":"jobs"})";
        f.close();
        LOG_INFO("Migrate", "Crawler config written: " + path);
        return true;
    }

    static std::string generateRandomPassword(int len) {
        static const char chars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%&*";
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, sizeof(chars) - 2);
        std::string s;
        s.reserve(len);
        for (int i = 0; i < len; ++i) s += chars[dis(gen)];
        return s;
    }
};
