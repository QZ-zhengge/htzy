import sys
from pathlib import Path
from datetime import datetime

# 添加项目根目录到Python路径
sys.path.append(str(Path(__file__).parent.parent.parent))

from backend.utils.database import db

class Interview:
    """面试模型类"""
    
    def __init__(self, id=None, user_id=None, job_position=None, start_time=None, end_time=None, status=None):
        self.id = id
        self.user_id = user_id
        self.job_position = job_position
        self.start_time = start_time
        self.end_time = end_time
        self.status = status
    
    def save(self):
        """保存面试信息到数据库"""
        if self.id is None:
            # 新增面试
            query = """
            INSERT INTO interviews (user_id, job_position, status)
            VALUES (%s, %s, %s)
            """
            params = (self.user_id, self.job_position, self.status or 'pending')
            
            if db.execute_query(query, params, commit=True):
                self.id = db.get_lastrowid()
                return True
            return False
        else:
            # 更新面试信息
            query = """
            UPDATE interviews
            SET job_position = %s, end_time = %s, status = %s
            WHERE id = %s AND user_id = %s
            """
            params = (self.job_position, self.end_time, self.status, self.id, self.user_id)
            
            return db.execute_query(query, params, commit=True)
    
    def start(self):
        """开始面试"""
        self.status = 'in_progress'
        return self.save()
    
    def end(self):
        """结束面试"""
        self.end_time = datetime.now()
        self.status = 'completed'
        return self.save()
    
    @classmethod
    def get_by_id(cls, interview_id):
        """根据ID获取面试信息"""
        query = "SELECT * FROM interviews WHERE id = %s"
        params = (interview_id,)
        
        if db.execute_query(query, params):
            result = db.fetch_one()
            if result:
                return cls(**result)
        return None
    
    @classmethod
    def get_by_user_id(cls, user_id):
        """根据用户ID获取面试列表"""
        query = "SELECT * FROM interviews WHERE user_id = %s ORDER BY start_time DESC"
        params = (user_id,)
        
        if db.execute_query(query, params):
            results = db.fetch_all()
            return [cls(**result) for result in results]
        return []

    @classmethod
    def get_active_by_user_id(cls, user_id):
        """获取用户正在进行的面试"""
        query = "SELECT * FROM interviews WHERE user_id = %s AND status = 'in_progress' LIMIT 1"
        params = (user_id,)

        if db.execute_query(query, params):
            result = db.fetch_one()
            if result:
                return cls(**result)
        return None

    @classmethod
    def get_completed_count(cls, user_id):
        """获取用户已完成面试数量"""
        query = "SELECT COUNT(*) as count FROM interviews WHERE user_id = %s AND status = 'completed'"
        params = (user_id,)

        if db.execute_query(query, params):
            result = db.fetch_one()
            return result['count'] if result else 0
        return 0

    @classmethod
    def get_pending_count(cls, user_id):
        """获取用户待完成面试数量（包括 pending 和 in_progress）"""
        query = "SELECT COUNT(*) as count FROM interviews WHERE user_id = %s AND status IN ('pending', 'in_progress')"
        params = (user_id,)

        if db.execute_query(query, params):
            result = db.fetch_one()
            return result['count'] if result else 0
        return 0


class InterviewQuestion:
    """面试问题模型类"""
    
    def __init__(self, id=None, interview_id=None, question_text=None, question_type=None, difficulty_level=None, order_number=None):
        self.id = id
        self.interview_id = interview_id
        self.question_text = question_text
        self.question_type = question_type
        self.difficulty_level = difficulty_level
        self.order_number = order_number
    
    def save(self):
        """保存面试问题到数据库"""
        if self.id is None:
            # 新增问题
            query = """
            INSERT INTO interview_questions (interview_id, question_text, question_type, difficulty_level, order_number)
            VALUES (%s, %s, %s, %s, %s)
            """
            params = (self.interview_id, self.question_text, self.question_type, self.difficulty_level, self.order_number)
            
            if db.execute_query(query, params, commit=True):
                self.id = db.get_lastrowid()
                return True
            return False
        return False
    
    @classmethod
    def get_by_interview_id(cls, interview_id):
        """根据面试ID获取问题列表"""
        query = """
        SELECT * FROM interview_questions
        WHERE interview_id = %s
        ORDER BY order_number ASC
        """
        params = (interview_id,)
        
        if db.execute_query(query, params):
            results = db.fetch_all()
            return [cls(**result) for result in results]
        return []
    
    @classmethod
    def get_by_id(cls, question_id):
        """根据ID获取问题信息"""
        query = "SELECT * FROM interview_questions WHERE id = %s"
        params = (question_id,)
        
        if db.execute_query(query, params):
            result = db.fetch_one()
            if result:
                return cls(**result)
        return None


class InterviewAnswer:
    """面试回答模型类"""
    
    def __init__(self, id=None, interview_id=None, question_id=None, answer_text=None, start_time=None, end_time=None):
        self.id = id
        self.interview_id = interview_id
        self.question_id = question_id
        self.answer_text = answer_text
        self.start_time = start_time
        self.end_time = end_time
    
    def save(self):
        """保存面试回答到数据库"""
        if self.id is None:
            # 新增回答
            query = """
            INSERT INTO interview_answers (interview_id, question_id, answer_text, start_time, end_time)
            VALUES (%s, %s, %s, %s, %s)
            """
            params = (self.interview_id, self.question_id, self.answer_text, self.start_time, self.end_time)
            
            if db.execute_query(query, params, commit=True):
                self.id = db.get_lastrowid()
                return True
            return False
        else:
            # 更新回答
            query = """
            UPDATE interview_answers
            SET answer_text = %s, end_time = %s
            WHERE id = %s AND interview_id = %s
            """
            params = (self.answer_text, self.end_time, self.id, self.interview_id)
            
            return db.execute_query(query, params, commit=True)
    
    @classmethod
    def get_by_interview_id(cls, interview_id):
        """根据面试ID获取回答列表"""
        query = """
        SELECT * FROM interview_answers
        WHERE interview_id = %s
        ORDER BY start_time ASC
        """
        params = (interview_id,)
        
        if db.execute_query(query, params):
            results = db.fetch_all()
            return [cls(**result) for result in results]
        return []
    
    @classmethod
    def get_by_question_id(cls, question_id):
        """根据问题ID获取回答信息"""
        query = "SELECT * FROM interview_answers WHERE question_id = %s"
        params = (question_id,)
        
        if db.execute_query(query, params):
            result = db.fetch_one()
            if result:
                return cls(**result)
        return None


class InterviewResult:
    """面试结果模型类"""
    
    def __init__(self, id=None, interview_id=None, professional_score=None, language_score=None, expression_score=None, overall_score=None, report=None, created_at=None):
        self.id = id
        self.interview_id = interview_id
        self.professional_score = professional_score
        self.language_score = language_score
        self.expression_score = expression_score
        self.overall_score = overall_score
        self.report = report
        self.created_at = created_at
    
    def save(self):
        """保存面试结果到数据库"""
        if self.id is None:
            # 新增结果
            query = """
            INSERT INTO interview_results (interview_id, professional_score, language_score, expression_score, overall_score, report)
            VALUES (%s, %s, %s, %s, %s, %s)
            """
            params = (self.interview_id, self.professional_score, self.language_score, self.expression_score, self.overall_score, self.report)
            
            if db.execute_query(query, params, commit=True):
                self.id = db.get_lastrowid()
                return True
            return False
        return False
    
    @classmethod
    def get_by_interview_id(cls, interview_id):
        """根据面试ID获取结果信息"""
        query = "SELECT * FROM interview_results WHERE interview_id = %s"
        params = (interview_id,)
        
        if db.execute_query(query, params):
            result = db.fetch_one()
            if result:
                return cls(**result)
        return None

    @classmethod
    def get_by_user_id(cls, user_id):
        """根据用户ID获取所有面试结果"""
        query = """
        SELECT ir.* FROM interview_results ir
        JOIN interviews i ON ir.interview_id = i.id
        WHERE i.user_id = %s
        ORDER BY ir.created_at DESC
        """
        params = (user_id,)

        if db.execute_query(query, params):
            results = db.fetch_all()
            return [cls(**result) for result in results]
        return []

    @classmethod
    def get_average_score_by_user_id(cls, user_id):
        """获取用户所有面试的平均分"""
        query = """
        SELECT AVG(ir.overall_score) as avg_score
        FROM interview_results ir
        JOIN interviews i ON ir.interview_id = i.id
        WHERE i.user_id = %s
        """
        params = (user_id,)

        if db.execute_query(query, params):
            result = db.fetch_one()
            return result['avg_score'] if result and result['avg_score'] is not None else 0
        return 0


class FacialAnalysis:
    """微表情分析模型类"""
    
    def __init__(self, id=None, interview_id=None, timestamp=None, frame_number=None, stress_level=None, dressing_neatness=None, eye_contact=None, facial_expression=None):
        self.id = id
        self.interview_id = interview_id
        self.timestamp = timestamp
        self.frame_number = frame_number
        self.stress_level = stress_level
        self.dressing_neatness = dressing_neatness
        self.eye_contact = eye_contact
        self.facial_expression = facial_expression
    
    def save(self):
        """保存微表情分析数据到数据库"""
        if self.id is None:
            # 新增分析数据
            query = """
            INSERT INTO facial_analysis (interview_id, timestamp, frame_number, 紧张程度, 着装整洁度, 眼神接触, 面部表情)
            VALUES (%s, %s, %s, %s, %s, %s, %s)
            """
            params = (self.interview_id, self.timestamp, self.frame_number, self.stress_level, self.dressing_neatness, self.eye_contact, self.facial_expression)
            
            if db.execute_query(query, params, commit=True):
                self.id = db.get_lastrowid()
                return True
            return False
        return False
    
    @classmethod
    def get_by_interview_id(cls, interview_id):
        """根据面试ID获取微表情分析数据"""
        query = """
        SELECT * FROM facial_analysis
        WHERE interview_id = %s
        ORDER BY timestamp ASC
        """
        params = (interview_id,)
        
        if db.execute_query(query, params):
            results = db.fetch_all()
            return [cls(**result) for result in results]
        return []
    
    @classmethod
    def get_average_scores(cls, interview_id):
        """获取面试微表情分析的平均分数"""
        query = """
        SELECT 
            AVG(紧张程度) as avg_stress,
            AVG(着装整洁度) as avg_dressing,
            AVG(眼神接触) as avg_eye_contact,
            AVG(面部表情) as avg_facial_expression
        FROM facial_analysis
        WHERE interview_id = %s
        """
        params = (interview_id,)
        
        if db.execute_query(query, params):
            result = db.fetch_one()
            return result
        return None


class VoiceAnalysis:
    """语音分析模型类"""
    
    def __init__(self, id=None, interview_id=None, timestamp=None, mandarin_level=None, voice_stability=None, answer_clarity=None, language_organization=None):
        self.id = id
        self.interview_id = interview_id
        self.timestamp = timestamp
        self.mandarin_level = mandarin_level
        self.voice_stability = voice_stability
        self.answer_clarity = answer_clarity
        self.language_organization = language_organization
    
    def save(self):
        """保存语音分析数据到数据库"""
        if self.id is None:
            # 新增分析数据
            query = """
            INSERT INTO voice_analysis (interview_id, timestamp, 普通话水平, 声音稳定性, 回答清晰度, 语言组织)
            VALUES (%s, %s, %s, %s, %s, %s)
            """
            params = (self.interview_id, self.timestamp, self.mandarin_level, self.voice_stability, self.answer_clarity, self.language_organization)
            
            if db.execute_query(query, params, commit=True):
                self.id = db.get_lastrowid()
                return True
            return False
        return False
    
    @classmethod
    def get_by_interview_id(cls, interview_id):
        """根据面试ID获取语音分析数据"""
        query = """
        SELECT * FROM voice_analysis
        WHERE interview_id = %s
        ORDER BY timestamp ASC
        """
        params = (interview_id,)
        
        if db.execute_query(query, params):
            results = db.fetch_all()
            return [cls(**result) for result in results]
        return []
    
    @classmethod
    def get_average_scores(cls, interview_id):
        """获取面试语音分析的平均分数"""
        query = """
        SELECT 
            AVG(普通话水平) as avg_mandarin,
            AVG(声音稳定性) as avg_stability,
            AVG(回答清晰度) as avg_clarity,
            AVG(语言组织) as avg_organization
        FROM voice_analysis
        WHERE interview_id = %s
        """
        params = (interview_id,)
        
        if db.execute_query(query, params):
            result = db.fetch_one()
            return result
        return None