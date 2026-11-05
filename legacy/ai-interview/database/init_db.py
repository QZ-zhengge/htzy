import mysql.connector
from mysql.connector import Error
import os
import sys
from pathlib import Path

# 添加项目根目录到Python路径
sys.path.append(str(Path(__file__).parent.parent))

from backend.config.config import DB_CONFIG

def create_database():
    """创建数据库（如果不存在）"""
    try:
        # 连接到MySQL服务器
        conn = mysql.connector.connect(
            host=DB_CONFIG['host'],
            user=DB_CONFIG['user'],
            password=DB_CONFIG['password'],
            port=DB_CONFIG['port']
        )
        
        if conn.is_connected():
            cursor = conn.cursor()
            # 创建数据库（如果不存在）
            cursor.execute(f"CREATE DATABASE IF NOT EXISTS {DB_CONFIG['database']} CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci")
            print(f"数据库 {DB_CONFIG['database']} 已创建或已存在")
            
            # 使用创建的数据库
            cursor.execute(f"USE {DB_CONFIG['database']}")
            
            # 创建用户表
            cursor.execute('''
            CREATE TABLE IF NOT EXISTS users (
                id INT PRIMARY KEY AUTO_INCREMENT,
                username VARCHAR(50) NOT NULL UNIQUE,
                password VARCHAR(100) NOT NULL,
                email VARCHAR(100) NOT NULL UNIQUE,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                last_login TIMESTAMP NULL
            )
            ''')
            
            # 创建简历表
            cursor.execute('''
            CREATE TABLE IF NOT EXISTS resumes (
                id INT PRIMARY KEY AUTO_INCREMENT,
                user_id INT NOT NULL,
                file_path VARCHAR(255) NOT NULL,
                content TEXT,
                upload_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
            )
            ''')
            
            # 创建面试记录表
            cursor.execute('''
            CREATE TABLE IF NOT EXISTS interviews (
                id INT PRIMARY KEY AUTO_INCREMENT,
                user_id INT NOT NULL,
                job_position VARCHAR(100) NOT NULL,
                start_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                end_time TIMESTAMP NULL,
                status ENUM('pending', 'in_progress', 'completed') DEFAULT 'pending',
                FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
            )
            ''')
            
            # 创建面试问题表
            cursor.execute('''
            CREATE TABLE IF NOT EXISTS interview_questions (
                id INT PRIMARY KEY AUTO_INCREMENT,
                interview_id INT NOT NULL,
                question_text TEXT NOT NULL,
                question_type VARCHAR(50) NOT NULL,
                difficulty_level INT NOT NULL,
                order_number INT NOT NULL,
                FOREIGN KEY (interview_id) REFERENCES interviews(id) ON DELETE CASCADE
            )
            ''')
            
            # 创建面试回答表
            cursor.execute('''
            CREATE TABLE IF NOT EXISTS interview_answers (
                id INT PRIMARY KEY AUTO_INCREMENT,
                interview_id INT NOT NULL,
                question_id INT NOT NULL,
                answer_text TEXT,
                start_time TIMESTAMP NOT NULL,
                end_time TIMESTAMP NOT NULL,
                FOREIGN KEY (interview_id) REFERENCES interviews(id) ON DELETE CASCADE,
                FOREIGN KEY (question_id) REFERENCES interview_questions(id) ON DELETE CASCADE
            )
            ''')
            
            # 创建面试结果表
            cursor.execute('''
            CREATE TABLE IF NOT EXISTS interview_results (
                id INT PRIMARY KEY AUTO_INCREMENT,
                interview_id INT NOT NULL,
                professional_score DECIMAL(5,2) NOT NULL,
                language_score DECIMAL(5,2) NOT NULL,
                expression_score DECIMAL(5,2) NOT NULL,
                overall_score DECIMAL(5,2) NOT NULL,
                report TEXT NOT NULL,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (interview_id) REFERENCES interviews(id) ON DELETE CASCADE
            )
            ''')
            
            # 创建微表情分析表
            cursor.execute('''
            CREATE TABLE IF NOT EXISTS facial_analysis (
                id INT PRIMARY KEY AUTO_INCREMENT,
                interview_id INT NOT NULL,
                timestamp TIMESTAMP NOT NULL,
                frame_number INT NOT NULL,
                紧张程度 DECIMAL(5,2) NOT NULL,
                着装整洁度 DECIMAL(5,2) NOT NULL,
                眼神接触 DECIMAL(5,2) NOT NULL,
                面部表情 DECIMAL(5,2) NOT NULL,
                FOREIGN KEY (interview_id) REFERENCES interviews(id) ON DELETE CASCADE
            )
            ''')
            
            # 创建语音分析表
            cursor.execute('''
            CREATE TABLE IF NOT EXISTS voice_analysis (
                id INT PRIMARY KEY AUTO_INCREMENT,
                interview_id INT NOT NULL,
                timestamp TIMESTAMP NOT NULL,
                普通话水平 DECIMAL(5,2) NOT NULL,
                声音稳定性 DECIMAL(5,2) NOT NULL,
                回答清晰度 DECIMAL(5,2) NOT NULL,
                语言组织 DECIMAL(5,2) NOT NULL,
                FOREIGN KEY (interview_id) REFERENCES interviews(id) ON DELETE CASCADE
            )
            ''')
            
            print("所有表已创建成功")
            conn.commit()
            
    except Error as e:
        print(f"数据库操作错误: {e}")
    finally:
        if conn.is_connected():
            cursor.close()
            conn.close()
            print("数据库连接已关闭")

if __name__ == "__main__":
    create_database()