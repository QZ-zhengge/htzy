import sys
from pathlib import Path
from datetime import datetime

# 添加项目根目录到Python路径
sys.path.append(str(Path(__file__).parent.parent.parent))

from backend.utils.database import db

class User:
    """用户模型类"""
    
    def __init__(self, id=None, username=None, password=None, email=None, created_at=None, last_login=None):
        self.id = id
        self.username = username
        self.password = password
        self.email = email
        self.created_at = created_at
        self.last_login = last_login
    
    def save(self):
        """保存用户信息到数据库"""
        if self.id is None:
            # 新增用户
            query = """
            INSERT INTO users (username, password, email)
            VALUES (%s, %s, %s)
            """
            params = (self.username, self.password, self.email)
            
            if db.execute_query(query, params, commit=True):
                self.id = db.get_lastrowid()
                return True
            return False
        else:
            # 更新用户信息
            query = """
            UPDATE users
            SET username = %s, password = %s, email = %s
            WHERE id = %s
            """
            params = (self.username, self.password, self.email, self.id)
            
            return db.execute_query(query, params, commit=True)
    
    def update_last_login(self):
        """更新最后登录时间"""
        query = """
        UPDATE users
        SET last_login = CURRENT_TIMESTAMP
        WHERE id = %s
        """
        params = (self.id,)
        
        return db.execute_query(query, params, commit=True)
    
    @classmethod
    def get_by_id(cls, user_id):
        """根据ID获取用户信息"""
        query = "SELECT * FROM users WHERE id = %s"
        params = (user_id,)
        
        if db.execute_query(query, params):
            result = db.fetch_one()
            if result:
                return cls(**result)
        return None
    
    @classmethod
    def get_by_username(cls, username):
        """根据用户名获取用户信息"""
        query = "SELECT * FROM users WHERE username = %s"
        params = (username,)
        
        if db.execute_query(query, params):
            result = db.fetch_one()
            if result:
                return cls(**result)
        return None
    
    @classmethod
    def get_by_email(cls, email):
        """根据邮箱获取用户信息"""
        query = "SELECT * FROM users WHERE email = %s"
        params = (email,)
        
        if db.execute_query(query, params):
            result = db.fetch_one()
            if result:
                return cls(**result)
        return None
    
    @classmethod
    def authenticate(cls, username, password):
        """用户认证"""
        query = "SELECT * FROM users WHERE username = %s AND password = %s"
        params = (username, password)
        
        if db.execute_query(query, params):
            result = db.fetch_one()
            if result:
                user = cls(**result)
                user.update_last_login()
                return user
        return None
    
    @classmethod
    def username_exists(cls, username):
        """检查用户名是否已存在"""
        query = "SELECT COUNT(*) as count FROM users WHERE username = %s"
        params = (username,)
        
        if db.execute_query(query, params):
            result = db.fetch_one()
            return result['count'] > 0
        return False
    
    @classmethod
    def email_exists(cls, email):
        """检查邮箱是否已存在"""
        query = "SELECT COUNT(*) as count FROM users WHERE email = %s"
        params = (email,)
        
        if db.execute_query(query, params):
            result = db.fetch_one()
            return result['count'] > 0
        return False