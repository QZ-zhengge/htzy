import sys
from pathlib import Path

# 添加项目根目录到Python路径
sys.path.append(str(Path(__file__).parent.parent.parent))

from backend.utils.database import db

class Resume:
    """简历模型类"""
    
    def __init__(self, id=None, user_id=None, file_path=None, content=None, upload_time=None):
        self.id = id
        self.user_id = user_id
        self.file_path = file_path
        self.content = content
        self.upload_time = upload_time
    
    def save(self):
        """保存简历信息到数据库"""
        if self.id is None:
            # 新增简历
            query = """
            INSERT INTO resumes (user_id, file_path, content)
            VALUES (%s, %s, %s)
            """
            params = (self.user_id, self.file_path, self.content)
            
            if db.execute_query(query, params, commit=True):
                self.id = db.get_lastrowid()
                return True
            return False
        else:
            # 更新简历信息
            query = """
            UPDATE resumes
            SET file_path = %s, content = %s
            WHERE id = %s AND user_id = %s
            """
            params = (self.file_path, self.content, self.id, self.user_id)
            
            return db.execute_query(query, params, commit=True)
    
    @classmethod
    def get_by_id(cls, resume_id):
        """根据ID获取简历信息"""
        query = "SELECT * FROM resumes WHERE id = %s"
        params = (resume_id,)
        
        if db.execute_query(query, params):
            result = db.fetch_one()
            if result:
                return cls(**result)
        return None
    
    @classmethod
    def get_by_user_id(cls, user_id):
        """根据用户ID获取简历列表"""
        query = "SELECT * FROM resumes WHERE user_id = %s ORDER BY upload_time DESC"
        params = (user_id,)
        
        if db.execute_query(query, params):
            results = db.fetch_all()
            return [cls(**result) for result in results]
        return []
    
    @classmethod
    def get_latest_by_user_id(cls, user_id):
        """获取用户最新上传的简历"""
        query = "SELECT * FROM resumes WHERE user_id = %s ORDER BY upload_time DESC LIMIT 1"
        params = (user_id,)
        
        if db.execute_query(query, params):
            result = db.fetch_one()
            if result:
                return cls(**result)
        return None
    
    @classmethod
    def delete(cls, resume_id, user_id):
        """删除简历"""
        query = "DELETE FROM resumes WHERE id = %s AND user_id = %s"
        params = (resume_id, user_id)
        
        return db.execute_query(query, params, commit=True)