import os
from dotenv import load_dotenv

# 加载环境变量
load_dotenv()

# 数据库配置
DB_CONFIG = {
    'host': os.getenv('DB_HOST', 'localhost'),
    'user': os.getenv('DB_USER', 'root'),
    'password': os.getenv('DB_PASSWORD', ''),
    'database': os.getenv('DB_NAME', 'ai_user'),
    'port': int(os.getenv('DB_PORT', '3306'))
}

# Minimax配置
VOLCENGINE_CONFIG = {
    'access_key': os.getenv('VOLCENGINE_ACCESS_KEY', 'CHANGE_ME'),
    'doubao_model': os.getenv('DOUBAO_MODEL', 'MiniMax-M2.7')  # Minimax模型名称
}

# 应用配置
APP_CONFIG = {
    'secret_key': os.getenv('APP_SECRET_KEY', ''),
    'debug': os.getenv('APP_DEBUG', 'True').lower() == 'true',
    'upload_folder': os.getenv('UPLOAD_FOLDER', 'uploads'),
    'max_content_length': int(os.getenv('MAX_CONTENT_LENGTH', '16777216')),  # 16MB
    'allowed_extensions': {'pdf', 'doc', 'docx', 'txt'}
}

# 面试配置
INTERVIEW_CONFIG = {
    'question_count': int(os.getenv('QUESTION_COUNT', '20')),
    'question_difficulty_levels': [1, 2, 3, 4, 5],  # 1-最简单，5-最难
    'question_types': ['technical', 'behavioral', 'situational', 'case_study', 'culture_fit'],
    'time_per_question': int(os.getenv('TIME_PER_QUESTION', '60')),  # 秒
    'frame_capture_interval': int(os.getenv('FRAME_CAPTURE_INTERVAL', '1000'))  # 毫秒
}

# 评分配置
SCORING_CONFIG = {
    'professional_weight': float(os.getenv('PROFESSIONAL_WEIGHT', '0.4')),
    'language_weight': float(os.getenv('LANGUAGE_WEIGHT', '0.3')),
    'expression_weight': float(os.getenv('EXPRESSION_WEIGHT', '0.3')),
    'pass_score': float(os.getenv('PASS_SCORE', '60.0'))
}