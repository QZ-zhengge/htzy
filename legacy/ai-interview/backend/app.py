import os
import sys
import json
import threading
import traceback
from datetime import datetime
from pathlib import Path
from flask import Flask, request, jsonify, session, redirect, url_for, render_template
from flask_cors import CORS
from werkzeug.utils import secure_filename

sys.path.append(str(Path(__file__).parent.parent))

from backend.config.config import APP_CONFIG, INTERVIEW_CONFIG, SCORING_CONFIG
from backend.models.user import User
from backend.models.resume import Resume
from backend.models.interview import (
    Interview, InterviewQuestion, InterviewAnswer,
    InterviewResult, FacialAnalysis, VoiceAnalysis
)

from backend.utils.volcengine_client_deepseek import VolcengineClientTextMixin

from backend.utils.image_analysis import VolcengineClientImageMixin

from database.init_db import create_database

volcengine_client = VolcengineClientTextMixin()
image_analysis_client = VolcengineClientImageMixin()
app = Flask(__name__, template_folder='../frontend/templates', static_folder='../frontend/static')
app.config['SECRET_KEY'] = APP_CONFIG['secret_key']
app.config['UPLOAD_FOLDER'] = APP_CONFIG['upload_folder']
app.config['MAX_CONTENT_LENGTH'] = APP_CONFIG['max_content_length']
CORS(app)

os.makedirs(app.config['UPLOAD_FOLDER'], exist_ok=True)

# 防重入：记录正在处理结束流程的面试ID，避免并发重复触发
_ending_interviews: set = set()
_ending_lock = threading.Lock()


def allowed_file(filename):
    return '.' in filename and filename.rsplit('.', 1)[1].lower() in APP_CONFIG['allowed_extensions']


def login_required(f):
    def decorated_function(*args, **kwargs):
        if 'user_id' not in session:
            print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 访问受保护路由 - 用户未登录")
            return jsonify({'error': '需要登录'}), 401
        return f(*args, **kwargs)

    decorated_function.__name__ = f.__name__
    return decorated_function


# ====================== 工具函数：AI 调用日志输出 ======================
def log_ai_start(name):
    print(f"\n📌 [{datetime.now().strftime('%H:%M:%S')}] 开始调用 AI → {name}")

def log_ai_success(name, result):
    print(f"✅ [{datetime.now().strftime('%H:%M:%S')}] AI 调用成功 → {name}")
    try:
        print(json.dumps(result, ensure_ascii=False, indent=2))
    except:
        print(result)

def log_ai_error(name, err):
    print(f"❌ [{datetime.now().strftime('%H:%M:%S')}] AI 调用失败 → {name}")
    print(f"错误信息：{str(err)}")
    traceback.print_exc()
# ======================================================================


# API路由 - 用户管理
@app.route('/api/users/register', methods=['POST'])
def register():
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 用户注册 - 开始")
    data = request.get_json()
    if not data.get('username') or not data.get('password') or not data.get('email'):
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 用户注册 - 失败 (缺少必填字段)")
        return jsonify({'error': '用户名、密码和邮箱不能为空'}), 400
    if User.username_exists(data['username']):
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 用户注册 - 失败 (用户名已存在: {data['username']})")
        return jsonify({'error': '用户名已存在'}), 400
    if User.email_exists(data['email']):
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 用户注册 - 失败 (邮箱已存在: {data['email']})")
        return jsonify({'error': '邮箱已存在'}), 400

    user = User(username=data['username'], password=data['password'], email=data['email'])
    if user.save():
        session['user_id'] = user.id
        session['username'] = user.username
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 用户注册 - 成功 (用户ID: {user.id}, 用户名: {user.username})")
        return jsonify({'message': '注册成功', 'user_id': user.id}), 201
    else:
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 用户注册 - 失败 (数据库保存失败)")
        return jsonify({'error': '注册失败'}), 500


@app.route('/api/users/login', methods=['POST'])
def login():
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 用户登录 - 开始")
    data = request.get_json()
    if not data.get('username') or not data.get('password'):
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 用户登录 - 失败 (缺少用户名或密码)")
        return jsonify({'error': '用户名和密码不能为空'}), 400
    user = User.authenticate(data['username'], data['password'])
    if user:
        session['user_id'] = user.id
        session['username'] = user.username
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 用户登录 - 成功 (用户ID: {user.id}, 用户名: {user.username})")
        return jsonify({'message': '登录成功', 'user_id': user.id}), 200
    else:
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 用户登录 - 失败 (用户名或密码错误: {data['username']})")
        return jsonify({'error': '用户名或密码错误'}), 401


@app.route('/api/users/logout', methods=['POST'])
@login_required
def logout():
    user_id = session.get('user_id')
    username = session.get('username')
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 用户登出 - 开始 (用户ID: {user_id}, 用户名: {username})")
    session.clear()
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 用户登出 - 成功")
    return jsonify({'message': '登出成功'}), 200


@app.route('/api/users/profile', methods=['GET'])
@login_required
def get_profile():
    user_id = session['user_id']
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 获取用户资料 - 开始 (用户ID: {user_id})")
    user = User.get_by_id(user_id)
    if user:
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 获取用户资料 - 成功 (用户名: {user.username})")
        return jsonify({
            'id': user.id, 'username': user.username, 'email': user.email,
            'created_at': user.created_at.strftime('%Y-%m-%d %H:%M:%S') if user.created_at else None,
            'last_login': user.last_login.strftime('%Y-%m-%d %H:%M:%S') if user.last_login else None
        }), 200
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 获取用户资料 - 失败 (用户不存在)")
    return jsonify({'error': '用户不存在'}), 404


@app.route('/api/user/interview-stats', methods=['GET'])
@login_required
def get_interview_stats():
    """获取用户面试统计数据"""
    user_id = session['user_id']
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 获取面试统计 - 开始 (用户ID: {user_id})")

    # 已完成面试数量
    completed = Interview.get_completed_count(user_id)
    # 待完成面试数量（包括 pending 和 in_progress）
    pending = Interview.get_pending_count(user_id)
    # 平均得分
    average_score = InterviewResult.get_average_score_by_user_id(user_id)

    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 获取面试统计 - 成功 (已完成: {completed}, 待完成: {pending}, 平均分: {average_score})")
    return jsonify({
        'completed': completed,
        'pending': pending,
        'averageScore': round(average_score, 1) if average_score else 0
    }), 200


@app.route('/api/user/interview-results', methods=['GET'])
@login_required
def get_user_interview_results():
    """获取用户所有面试结果"""
    user_id = session['user_id']
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 获取面试结果列表 - 开始 (用户ID: {user_id})")

    # 获取用户所有面试结果
    results = InterviewResult.get_by_user_id(user_id)
    results_list = []

    for result in results:
        # 获取对应的面试信息
        interview = Interview.get_by_id(result.interview_id)
        if interview:
            results_list.append({
                'interviewId': interview.id,
                'jobPosition': interview.job_position,
                'date': result.created_at.strftime('%Y-%m-%d %H:%M:%S') if result.created_at else None,
                'professionalScore': result.professional_score or 0,
                'languageScore': result.language_score or 0,
                'expressionScore': result.expression_score or 0,
                'totalScore': result.overall_score or 0
            })

    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 获取面试结果列表 - 成功 (数量: {len(results_list)})")
    return jsonify(results_list), 200


# API路由 - 简历管理
@app.route('/api/resumes', methods=['POST'])
@login_required
def upload_resume():
    user_id = session['user_id']
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 上传简历 - 开始 (用户ID: {user_id})")
    if 'file' not in request.files:
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 上传简历 - 失败 (无文件)")
        return jsonify({'error': '没有文件上传'}), 400
    file = request.files['file']
    if file.filename == '':
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 上传简历 - 失败 (空文件名)")
        return jsonify({'error': '没有选择文件'}), 400
    if not allowed_file(file.filename):
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 上传简历 - 失败 (不支持的文件类型: {file.filename})")
        return jsonify({'error': '不支持的文件类型'}), 400

    filename = secure_filename(file.filename)
    filepath = os.path.join(app.config['UPLOAD_FOLDER'], filename)
    file.save(filepath)
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 上传简历 - 文件已保存: {filepath}")

    content = ""
    try:
        if filename.endswith('.txt'):
            with open(filepath, 'r', encoding='utf-8') as f:
                content = f.read()
            print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 上传简历 - 读取文本内容成功，长度: {len(content)}")
        else:
            content = f"文件已上传: {filename}"
            print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 上传简历 - 非文本文件，仅记录文件名")
    except Exception as e:
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 上传简历 - 读取文件错误: {str(e)}")

    resume = Resume(user_id=user_id, file_path=filepath, content=content)
    if resume.save():
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 上传简历 - 成功 (简历ID: {resume.id})")
        return jsonify({'message': '简历上传成功', 'resume_id': resume.id}), 201
    else:
        if os.path.exists(filepath):
            os.remove(filepath)
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 上传简历 - 失败 (数据库保存失败)")
        return jsonify({'error': '简历保存失败'}), 500


@app.route('/api/resumes', methods=['GET'])
@login_required
def get_resumes():
    user_id = session['user_id']
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 获取简历列表 - 开始 (用户ID: {user_id})")
    resumes = Resume.get_by_user_id(user_id)
    count = len(resumes)
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 获取简历列表 - 成功 (数量: {count})")
    return jsonify([{
        'id': r.id, 'file_path': r.file_path,
        'upload_time': r.upload_time.strftime('%Y-%m-%d %H:%M:%S') if r.upload_time else None
    } for r in resumes]), 200


@app.route('/api/resumes/<int:resume_id>', methods=['GET'])
@login_required
def get_resume(resume_id):
    user_id = session['user_id']
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 获取简历详情 - 开始 (用户ID: {user_id}, 简历ID: {resume_id})")
    resume = Resume.get_by_id(resume_id)
    if resume and resume.user_id == user_id:
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 获取简历详情 - 成功")
        return jsonify({
            'id': resume.id, 'file_path': resume.file_path, 'content': resume.content,
            'upload_time': resume.upload_time.strftime('%Y-%m-%d %H:%M:%S') if resume.upload_time else None
        }), 200
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 获取简历详情 - 失败 (无权访问或不存在)")
    return jsonify({'error': '简历不存在或无权访问'}), 404


@app.route('/api/resumes/<int:resume_id>', methods=['DELETE'])
@login_required
def delete_resume(resume_id):
    user_id = session['user_id']
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 删除简历 - 开始 (用户ID: {user_id}, 简历ID: {resume_id})")
    resume = Resume.get_by_id(resume_id)
    if not resume or resume.user_id != user_id:
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 删除简历 - 失败 (无权访问或不存在)")
        return jsonify({'error': '简历不存在或无权访问'}), 404
    if os.path.exists(resume.file_path):
        os.remove(resume.file_path)
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 删除简历 - 文件已删除: {resume.file_path}")
    if Resume.delete(resume_id, user_id):
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 删除简历 - 成功")
        return jsonify({'message': '简历删除成功'}), 200
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 删除简历 - 失败 (数据库删除失败)")
    return jsonify({'error': '简历删除失败'}), 500


# API路由 - 面试管理
@app.route('/api/interviews', methods=['POST'])
@login_required
def create_interview():
    user_id = session['user_id']
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 创建面试 - 开始 (用户ID: {user_id})")
    data = request.get_json()
    if not data.get('job_position'):
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 创建面试 - 失败 (缺少岗位名称)")
        return jsonify({'error': '岗位名称不能为空'}), 400

    resume = Resume.get_latest_by_user_id(user_id)
    if not resume:
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 创建面试 - 失败 (用户无简历)")
        return jsonify({'error': '请先上传简历'}), 400

    interview = Interview(user_id=user_id, job_position=data['job_position'], status='pending')
    if not interview.save():
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 创建面试 - 失败 (数据库保存失败)")
        return jsonify({'error': '面试创建失败'}), 500

    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 创建面试 - 面试记录已创建 (面试ID: {interview.id})")

    # ====================== AI：分析简历 ======================
    try:
        log_ai_start("分析简历")
        resume_info = volcengine_client.analyze_resume(resume.content, data['job_position'])
        log_ai_success("分析简历", resume_info)
    except Exception as e:
        log_ai_error("分析简历", e)
        resume_info = {
            "基本信息": {"姓名": "未知", "联系方式": "未知"},
            "工作经验摘要": "", "教育背景摘要": "", "技能清单": [],
            "项目经验摘要": "", "与目标岗位匹配点": [], "可能的面试问题点": []
        }

    # ====================== AI：生成题目 ======================
    try:
        log_ai_start("生成面试题目")
        questions_data = volcengine_client.generate_interview_questions(
            resume_info, data['job_position'], INTERVIEW_CONFIG['question_count']
        )
        log_ai_success("生成面试题目", questions_data)
    except Exception as e:
        log_ai_error("生成面试题目", e)
        questions_data = None

    if questions_data and 'questions' in questions_data:
        q_count = len(questions_data['questions'])
        for i, q in enumerate(questions_data['questions']):
            question = InterviewQuestion(
                interview_id=interview.id, question_text=q['text'],
                question_type=q['type'], difficulty_level=q['difficulty'], order_number=i + 1
            )
            question.save()
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 创建面试 - 已生成 {q_count} 道面试题目")
    else:
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 创建面试 - 未能生成题目")

    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 创建面试 - 成功 (面试ID: {interview.id})")
    return jsonify({'message': '面试创建成功', 'interview_id': interview.id}), 201


@app.route('/api/interviews', methods=['GET'])
@login_required
def get_interviews():
    user_id = session['user_id']
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 获取面试列表 - 开始 (用户ID: {user_id})")
    interviews = Interview.get_by_user_id(user_id)
    count = len(interviews)
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 获取面试列表 - 成功 (数量: {count})")
    return jsonify([{
        'id': i.id, 'job_position': i.job_position,
        'start_time': i.start_time.strftime('%Y-%m-%d %H:%M:%S') if i.start_time else None,
        'end_time': i.end_time.strftime('%Y-%m-%d %H:%M:%S') if i.end_time else None,
        'status': i.status
    } for i in interviews]), 200


@app.route('/api/interviews/<int:interview_id>', methods=['GET'])
@login_required
def get_interview(interview_id):
    user_id = session['user_id']
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 获取面试详情 - 开始 (用户ID: {user_id}, 面试ID: {interview_id})")
    interview = Interview.get_by_id(interview_id)
    if not interview or interview.user_id != user_id:
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 获取面试详情 - 失败 (无权访问或不存在)")
        return jsonify({'error': '面试不存在或无权访问'}), 404

    questions = InterviewQuestion.get_by_interview_id(interview_id)
    result = InterviewResult.get_by_interview_id(interview_id)

    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 获取面试详情 - 成功 (题目数: {len(questions)}, 有结果: {result is not None})")
    return jsonify({
        'id': interview.id, 'job_position': interview.job_position,
        'start_time': interview.start_time.strftime('%Y-%m-%d %H:%M:%S') if interview.start_time else None,
        'end_time': interview.end_time.strftime('%Y-%m-%d %H:%M:%S') if interview.end_time else None,
        'status': interview.status,
        'questions': [{
            'id': q.id, 'text': q.question_text, 'type': q.question_type,
            'difficulty': q.difficulty_level, 'order': q.order_number
        } for q in questions],
        'result': {
            'professional_score': result.professional_score,
            'language_score': result.language_score,
            'expression_score': result.expression_score,
            'overall_score': result.overall_score,
            'report': result.report
        } if result else None
    }), 200


@app.route('/api/interviews/<int:interview_id>/start', methods=['POST'])
@login_required
def start_interview(interview_id):
    user_id = session['user_id']
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 开始面试 - 开始 (用户ID: {user_id}, 面试ID: {interview_id})")
    interview = Interview.get_by_id(interview_id)
    if not interview or interview.user_id != user_id:
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 开始面试 - 失败 (无权访问或不存在)")
        return jsonify({'error': '面试不存在或无权访问'}), 404
    if interview.status != 'pending':
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 开始面试 - 失败 (状态不正确: {interview.status})")
        return jsonify({'error': '面试状态不正确'}), 400
    if interview.start():
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 开始面试 - 成功")
        return jsonify({'message': '面试已开始'}), 200
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 开始面试 - 失败 (数据库更新失败)")
    return jsonify({'error': '面试开始失败'}), 500


@app.route('/api/interviews/<int:interview_id>/end', methods=['POST'])
@login_required
def end_interview(interview_id):
    user_id = session['user_id']
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 结束面试 - 开始 (用户ID: {user_id}, 面试ID: {interview_id})")

    # 防重入：同一面试的结束请求只允许处理一次
    with _ending_lock:
        if interview_id in _ending_interviews:
            print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 结束面试 - 忽略重复请求 (面试ID: {interview_id})")
            return jsonify({'error': '面试正在结束处理中，请勿重复提交'}), 429
        _ending_interviews.add(interview_id)

    try:
        interview = Interview.get_by_id(interview_id)

        if not interview or interview.user_id != user_id:
            print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 结束面试 - 失败 (无权访问或不存在)")
            return jsonify({'error': '面试不存在或无权访问'}), 404

        if interview.status not in ['pending', 'in_progress']:
            print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 结束面试 - 失败 (状态不正确: {interview.status})")
            return jsonify({'error': '面试状态不正确'}), 400

        # 调用 end() 更新面试状态为 completed
        if not interview.end():
            print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 结束面试 - 失败 (更新面试状态失败)")
            return jsonify({'error': '面试结束失败'}), 500

        questions = InterviewQuestion.get_by_interview_id(interview_id)
        answers = InterviewAnswer.get_by_interview_id(interview_id)

        facial_scores = FacialAnalysis.get_average_scores(interview_id) or {}
        voice_scores = VoiceAnalysis.get_average_scores(interview_id) or {}

        # 递归转换 Decimal 为 float，避免 json.dumps 报 TypeError
        def _decimal_to_float(obj):
            import decimal
            if isinstance(obj, decimal.Decimal):
                return float(obj)
            if isinstance(obj, dict):
                return {k: _decimal_to_float(v) for k, v in obj.items()}
            if isinstance(obj, (list, tuple)):
                return [_decimal_to_float(v) for v in obj]
            return obj
        facial_scores = _decimal_to_float(facial_scores)
        voice_scores = _decimal_to_float(voice_scores)

        professional_scores = []
        for question, answer in zip(questions, answers):
            try:
                log_ai_start(f"分析问题回答：{question.question_text[:20]}...")
                analysis = volcengine_client.analyze_answer(
                    question.question_text,
                    answer.answer_text,
                    {},
                    interview.job_position
                )
                log_ai_success(f"分析问题回答：{question.question_text[:20]}...", analysis)

                if analysis and '专业知识分' in analysis:
                    professional_scores.append(analysis['专业知识分'])
            except Exception as e:
                log_ai_error(f"分析问题回答：{question.question_text[:20]}...", e)

        professional_score = sum(professional_scores) / len(professional_scores) if professional_scores else 0

        # 安全计算表情管理分数
        expression_score = 0
        if facial_scores:
            stress = float(facial_scores.get('avg_stress') or 0)
            dressing = float(facial_scores.get('avg_dressing') or 0)
            eye = float(facial_scores.get('avg_eye_contact') or 0)
            facial = float(facial_scores.get('avg_facial_expression') or 0)
            expression_score = (stress + dressing + eye + facial) / 4

        # 安全计算语言表达分数
        language_score = 0
        if voice_scores:
            mandarin = float(voice_scores.get('avg_mandarin') or 0)
            stability = float(voice_scores.get('avg_stability') or 0)
            clarity = float(voice_scores.get('avg_clarity') or 0)
            organization = float(voice_scores.get('avg_organization') or 0)
            language_score = (mandarin + stability + clarity + organization) / 4

        # 统一转为 float，防止 decimal.Decimal 与 float 权重相乘报 TypeError
        professional_score = float(professional_score)
        language_score = float(language_score)
        expression_score = float(expression_score)

        overall_score = (
                professional_score * SCORING_CONFIG['professional_weight'] +
                language_score * SCORING_CONFIG['language_weight'] +
                expression_score * SCORING_CONFIG['expression_weight']
        )

        interview_data = {
            'job_position': interview.job_position,
            'start_time': interview.start_time.strftime('%Y-%m-%d %H:%M:%S') if interview.start_time else None,
            'end_time': interview.end_time.strftime('%Y-%m-%d %H:%M:%S') if interview.end_time else None
        }

        try:
            log_ai_start("生成面试报告")
            report = volcengine_client.generate_interview_report(
                interview_data,
                facial_scores,
                voice_scores,
                {'average': professional_score, 'details': professional_scores}
            )
            # 第一次调用失败时重试一次
            if report is None:
                print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 生成面试报告第一次失败，重试...")
                report = volcengine_client.generate_interview_report(
                    interview_data,
                    facial_scores,
                    voice_scores,
                    {'average': professional_score, 'details': professional_scores}
                )
            if report is not None:
                log_ai_success("生成面试报告", report)
            else:
                print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 生成面试报告两次均失败，使用默认报告")
        except Exception as e:
            log_ai_error("生成面试报告", e)
            report = None

        if report is None:
            report = {
                "面试概况": "本次面试已完成，AI报告生成服务暂时不可用，分数已正常保存。",
                "各项能力评估": {},
                "优势": ["已完成全部面试题目"],
                "不足": ["暂无AI分析数据"],
                "改进建议": ["请重新进入结果页查看，若报告仍未生成请联系管理员"],
                "总体评价": f"综合得分：{overall_score:.1f}分"
            }

        report_str = json.dumps(report, ensure_ascii=False, indent=2) if isinstance(report, dict) else str(report)

        result = InterviewResult(
            interview_id=interview_id,
            professional_score=professional_score,
            language_score=language_score,
            expression_score=expression_score,
            overall_score=overall_score,
            report=report_str
        )

        if not result.save():
            print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 结束面试 - 失败 (保存结果失败)")
            return jsonify({'error': '保存面试结果失败'}), 500

        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 结束面试 - 成功 (结果ID: {result.id}, 总分: {overall_score:.2f})")
        return jsonify({'message': '面试已结束', 'result_id': result.id}), 200

    except Exception as e:
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 结束面试 - 异常: {str(e)}")
        traceback.print_exc()
        return jsonify({'error': '面试结束失败'}), 500
    finally:
        # 无论成功或失败，都释放防重入锁
        with _ending_lock:
            _ending_interviews.discard(interview_id)


# API路由 - 问题和回答管理
@app.route('/api/interviews/<int:interview_id>/questions', methods=['GET'])
@login_required
def get_interview_questions(interview_id):
    user_id = session['user_id']
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 获取面试题目 - 开始 (用户ID: {user_id}, 面试ID: {interview_id})")
    interview = Interview.get_by_id(interview_id)
    if not interview or interview.user_id != user_id:
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 获取面试题目 - 失败 (无权访问或不存在)")
        return jsonify({'error': '面试不存在或无权访问'}), 404
    questions = InterviewQuestion.get_by_interview_id(interview_id)
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 获取面试题目 - 成功 (题目数: {len(questions)})")
    return jsonify([{
        'id': q.id, 'text': q.question_text, 'type': q.question_type,
        'difficulty': q.difficulty_level, 'order': q.order_number
    } for q in questions]), 200


@app.route('/api/interviews/<int:interview_id>/analyze_face', methods=['POST'])
@login_required
def analyze_face(interview_id):
    user_id = session['user_id']
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 面部表情分析 - 开始 (用户ID: {user_id}, 面试ID: {interview_id})")

    data = request.get_json()
    if not data:
        return jsonify({'error': '请求体为空'}), 400

    required_fields = ['frame_number', 'timestamp', 'image']
    for field in required_fields:
        if field not in data:
            return jsonify({'error': f'{field}不能为空'}), 400

    # 获取面试信息，若查询失败则返回 404
    try:
        interview = Interview.get_by_id(interview_id)
    except Exception as e:
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 数据库查询异常: {str(e)}")
        return jsonify({'error': '服务器内部错误'}), 500

    if not interview or interview.user_id != user_id:
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 面部表情分析 - 失败 (无权访问或面试不存在)")
        return jsonify({'error': '面试不存在或无权访问'}), 404

    # 如果面试已结束，直接返回空结果（避免错误）
    if interview.status != 'in_progress':
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 面部表情分析 - 面试已结束，忽略帧")
        return jsonify({"紧张程度": 0, "着装整洁度": 0, "眼神接触": 0, "面部表情": 0}), 200

    image_data = data['image']
    frame_number = data['frame_number']
    timestamp = data['timestamp']

    try:
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 面部表情分析 - 调用AI (帧号: {frame_number})")
        analysis_result = image_analysis_client.analyze_facial_expression(image_data)

        if analysis_result is None:
            print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 面部表情分析 - AI调用失败 (返回None)")
            return jsonify({'error': 'AI分析失败'}), 500

        # 确保返回的是字典
        if isinstance(analysis_result, str):
            try:
                analysis_result = json.loads(analysis_result)
            except json.JSONDecodeError:
                print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 面部表情分析 - AI返回非JSON格式: {analysis_result}")
                return jsonify({'error': 'AI返回数据格式错误'}), 500

        # 补全缺失字段
        expected_keys = ['紧张程度', '着装整洁度', '眼神接触', '面部表情']
        for key in expected_keys:
            if key not in analysis_result:
                analysis_result[key] = 0

        # 保存到数据库（带重试机制，避免因连接问题影响返回）
        save_success = False
        for attempt in range(2):  # 最多重试2次
            try:
                if isinstance(timestamp, (int, float)):
                    dt = datetime.fromtimestamp(timestamp / 1000)
                else:
                    dt = datetime.now()

                fa = FacialAnalysis(
                    interview_id=interview_id,
                    timestamp=dt,
                    frame_number=frame_number,
                    stress_level=analysis_result.get('紧张程度', 0),
                    dressing_neatness=analysis_result.get('着装整洁度', 0),
                    eye_contact=analysis_result.get('眼神接触', 0),
                    facial_expression=analysis_result.get('面部表情', 0)
                )
                fa.save()
                save_success = True
                print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 微表情分析记录已保存 (ID: {fa.id})")
                break
            except Exception as e:
                print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 保存微表情分析记录失败 (尝试 {attempt+1}): {str(e)}")
                if attempt == 0:
                    # 第一次失败后，尝试重新获取数据库连接（如果有需要）
                    # 这里可以加一个短暂的等待
                    import time
                    time.sleep(0.5)

        if not save_success:
            print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 最终保存失败，数据未入库")

        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 面部表情分析 - 成功 (帧号: {frame_number})")
        return jsonify(analysis_result), 200

    except Exception as e:
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 面部表情分析 - 异常: {str(e)}")
        traceback.print_exc()
        return jsonify({'error': '服务器内部错误'}), 500



@app.route('/api/interviews/<int:interview_id>/answers', methods=['POST'])
@login_required
def submit_answer(interview_id):
    user_id = session['user_id']
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 提交回答 - 开始 (用户ID: {user_id}, 面试ID: {interview_id})")
    data = request.get_json()
    if not data.get('question_id') or not data.get('answer_text'):
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 提交回答 - 失败 (缺少问题ID或回答内容)")
        return jsonify({'error': '问题ID和回答内容不能为空'}), 400
    interview = Interview.get_by_id(interview_id)
    if not interview or interview.user_id != user_id:
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 提交回答 - 失败 (无权访问或不存在)")
        return jsonify({'error': '面试不存在或无权访问'}), 404
    if interview.status != 'in_progress':
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 提交回答 - 失败 (面试状态不正确: {interview.status})")
        return jsonify({'error': '面试状态不正确'}), 400
    question = InterviewQuestion.get_by_id(data['question_id'])
    if not question or question.interview_id != interview_id:
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 提交回答 - 失败 (问题不存在或不属于该面试)")
        return jsonify({'error': '问题不存在或不属于该面试'}), 400
    existing_answer = InterviewAnswer.get_by_question_id(data['question_id'])
    if existing_answer:
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 提交回答 - 失败 (该问题已有回答)")
        return jsonify({'error': '该问题已有回答'}), 400

    answer = InterviewAnswer(
        interview_id=interview_id, question_id=data['question_id'],
        answer_text=data['answer_text'], start_time=datetime.now(), end_time=datetime.now()
    )
    if answer.save():
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 提交回答 - 成功 (回答ID: {answer.id})")
        return jsonify({'message': '回答提交成功', 'answer_id': answer.id}), 201
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 提交回答 - 失败 (数据库保存失败)")
    return jsonify({'error': '回答保存失败'}), 500


# API路由 - 分析数据提交
@app.route('/api/interviews/<int:interview_id>/facial-analysis', methods=['POST'])
@login_required
def submit_facial_analysis(interview_id):
    user_id = session['user_id']
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 提交微表情分析 - 开始 (用户ID: {user_id}, 面试ID: {interview_id})")
    data = request.get_json()
    required = ['timestamp', 'frame_number', '紧张程度', '着装整洁度', '眼神接触', '面部表情']
    for f in required:
        if f not in data:
            print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 提交微表情分析 - 失败 (缺少字段: {f})")
            return jsonify({'error': f'{f}不能为空'}), 400
    interview = Interview.get_by_id(interview_id)
    if not interview or interview.user_id != user_id:
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 提交微表情分析 - 失败 (无权访问或不存在)")
        return jsonify({'error': '面试不存在或无权访问'}), 404
    if interview.status != 'in_progress':
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 提交微表情分析 - 失败 (面试状态不正确: {interview.status})")
        return jsonify({'error': '面试状态不正确'}), 400

    fa = FacialAnalysis(
        interview_id=interview_id, timestamp=datetime.now(), frame_number=data['frame_number'],
        stress_level=data['紧张程度'], dressing_neatness=data['着装整洁度'],
        eye_contact=data['眼神接触'], facial_expression=data['面部表情']
    )
    if fa.save():
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 提交微表情分析 - 成功")
        return jsonify({'message': '微表情分析数据提交成功'}), 201
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 提交微表情分析 - 失败 (数据库保存失败)")
    return jsonify({'error': '数据保存失败'}), 500


@app.route('/api/interviews/<int:interview_id>/voice-analysis', methods=['POST'])
@login_required
def submit_voice_analysis(interview_id):
    user_id = session['user_id']
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 提交语音分析 - 开始 (用户ID: {user_id}, 面试ID: {interview_id})")
    data = request.get_json()
    required = ['timestamp', '普通话水平', '声音稳定性', '回答清晰度', '语言组织']
    for f in required:
        if f not in data:
            print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 提交语音分析 - 失败 (缺少字段: {f})")
            return jsonify({'error': f'{f}不能为空'}), 400
    interview = Interview.get_by_id(interview_id)
    if not interview or interview.user_id != user_id:
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 提交语音分析 - 失败 (无权访问或不存在)")
        return jsonify({'error': '面试不存在或无权访问'}), 404
    if interview.status != 'in_progress':
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 提交语音分析 - 失败 (面试状态不正确: {interview.status})")
        return jsonify({'error': '面试状态不正确'}), 400

    va = VoiceAnalysis(
        interview_id=interview_id, timestamp=datetime.now(), mandarin_level=data['普通话水平'],
        voice_stability=data['声音稳定性'], answer_clarity=data['回答清晰度'],
        language_organization=data['语言组织']
    )
    if va.save():
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 提交语音分析 - 成功")
        return jsonify({'message': '语音分析数据提交成功'}), 201
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 提交语音分析 - 失败 (数据库保存失败)")
    return jsonify({'error': '数据保存失败'}), 500


# API路由 - 面试结果
@app.route('/api/interviews/<int:interview_id>/results', methods=['GET'])
@login_required
def get_interview_results(interview_id):
    user_id = session['user_id']
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 获取面试结果 - 开始 (用户ID: {user_id}, 面试ID: {interview_id})")
    interview = Interview.get_by_id(interview_id)
    if not interview or interview.user_id != user_id:
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 获取面试结果 - 失败 (无权访问或不存在)")
        return jsonify({'error': '面试不存在或无权访问'}), 404

    result = InterviewResult.get_by_interview_id(interview_id)
    if not result:
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 获取面试结果 - 失败 (结果不存在)")
        return jsonify({'error': '面试结果不存在'}), 404

    try:
        duration = None
        if interview.start_time and interview.end_time:
            delta = interview.end_time - interview.start_time
            duration = f"{delta.seconds // 60}分钟"
    except:
        duration = "未知时长"

    # 递归转换 Decimal 为 float，确保 jsonify 正常工作
    def _decimal_to_float(obj):
        import decimal
        if isinstance(obj, decimal.Decimal):
            return float(obj)
        if isinstance(obj, dict):
            return {k: _decimal_to_float(v) for k, v in obj.items()}
        if isinstance(obj, (list, tuple)):
            return [_decimal_to_float(v) for v in obj]
        return obj

    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 获取面试结果 - 成功 (总分: {result.overall_score})")
    result_data = {
        "professional_score": result.professional_score or 0,
        "language_score": result.language_score or 0,
        "expression_score": result.expression_score or 0,
        "overall_score": result.overall_score or 0,
        "report": result.report or "",
        "created_at": result.created_at.strftime('%Y-%m-%d %H:%M:%S') if result.created_at else "未知时间",
        "job_position": interview.job_position or "未知岗位",
        "duration": duration,
        "question_count": len(InterviewQuestion.get_by_interview_id(interview_id)) or 0
    }
    return jsonify(_decimal_to_float(result_data)), 200


# 前端页面路由
@app.route('/')
def index():
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 访问首页")
    return render_template('index.html')

@app.route('/login')
def login_page():
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 访问登录页")
    return render_template('login.html')

@app.route('/register')
def register_page():
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 访问注册页")
    return render_template('register.html')

@app.route('/dashboard')
@login_required
def dashboard():
    user_id = session['user_id']
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 访问仪表盘 (用户ID: {user_id})")
    return render_template('dashboard.html')

@app.route('/resume')
@login_required
def resume_page():
    user_id = session['user_id']
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 访问简历管理页 (用户ID: {user_id})")
    return render_template('resume.html')

@app.route('/interview/<int:interview_id>')
@login_required
def interview_page(interview_id):
    user_id = session['user_id']
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 访问面试页 (用户ID: {user_id}, 面试ID: {interview_id})")
    return render_template('interview.html', interview_id=interview_id)

@app.route('/results/<int:interview_id>')
@login_required
def results_page(interview_id):
    user_id = session['user_id']
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 操作: 访问结果页 (用户ID: {user_id}, 面试ID: {interview_id})")
    return render_template('results.html', interview_id=interview_id)


if __name__ == '__main__':
    create_database()
    app.run(debug=APP_CONFIG['debug'], host='0.0.0.0', port=5000)