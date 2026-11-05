import mysql.connector
from mysql.connector import Error
import sys
from pathlib import Path

# 添加项目根目录到Python路径
sys.path.append(str(Path(__file__).parent.parent.parent))

from backend.config.config import DB_CONFIG

class DatabaseConnection:
    """数据库连接管理类"""
    
    def __init__(self):
        self.connection = None
        self.cursor = None
    
    def connect(self):
        """建立数据库连接"""
        try:
            self.connection = mysql.connector.connect(
                host=DB_CONFIG['host'],
                user=DB_CONFIG['user'],
                password=DB_CONFIG['password'],
                database=DB_CONFIG['database'],
                port=DB_CONFIG['port']
            )
            if self.connection.is_connected():
                self.cursor = self.connection.cursor(dictionary=True)
                return True
        except Error as e:
            print(f"数据库连接错误: {e}")
            return False
    
    def disconnect(self):
        """关闭数据库连接"""
        try:
            if self.cursor:
                self.cursor.close()
        except:
            pass
        try:
            if self.connection and self.connection.is_connected():
                self.connection.close()
        except:
            pass
        self.connection = None
        self.cursor = None
    
    def execute_query(self, query, params=None, commit=False):
        """执行SQL查询，带自动重连机制"""
        max_retries = 2
        for attempt in range(max_retries):
            try:
                if not self.connection or not self.connection.is_connected():
                    self.connect()

                if params:
                    self.cursor.execute(query, params)
                else:
                    self.cursor.execute(query)

                if commit:
                    self.connection.commit()

                return True
            except Error as e:
                error_code = e.errno if hasattr(e, 'errno') else 0
                # 检测连接丢失错误，尝试重连
                if error_code in (2013, 2006, 2003) and attempt < max_retries - 1:
                    print(f"数据库连接异常 (错误码: {error_code})，尝试重连...")
                    self.disconnect()
                    self.connect()
                    continue
                print(f"查询执行错误: {e}")
                return False
        return False
    
    def fetch_all(self):
        """获取所有查询结果"""
        return self.cursor.fetchall()
    
    def fetch_one(self):
        """获取单条查询结果"""
        return self.cursor.fetchone()
    
    def get_lastrowid(self):
        """获取最后插入记录的ID"""
        return self.cursor.lastrowid
    
    def execute_transaction(self, queries):
        """执行事务"""
        try:
            if not self.connection or not self.connection.is_connected():
                self.connect()
            
            # 开始事务
            self.connection.start_transaction()
            
            try:
                for query, params in queries:
                    if params:
                        self.cursor.execute(query, params)
                    else:
                        self.cursor.execute(query)
                
                # 提交事务
                self.connection.commit()
                return True
            except Error as e:
                # 回滚事务
                self.connection.rollback()
                print(f"事务执行错误: {e}")
                return False
        except Error as e:
            print(f"事务处理错误: {e}")
            return False

# 创建数据库连接实例
db = DatabaseConnection()