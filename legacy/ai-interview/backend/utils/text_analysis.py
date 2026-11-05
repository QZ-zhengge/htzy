import json
import hashlib
import hmac
import base64
import threading
import ssl
from datetime import datetime
from time import mktime
from urllib.parse import urlparse, urlencode
from wsgiref.handlers import format_date_time
import websocket

# 全局变量，用于存储流式回答
answer = ""
sid = ''

class VolcengineClientTextMixin:
    """文本分析混合类，使用科大讯飞星火大模型API（WebSocket版）"""

    def __init__(self):
        # ===================== 配置已全部写死在代码里 =====================
        self.app_id = "CHANGE_ME"
        self.api_key = "CHANGE_ME"
        self.api_secret = "CHANGE_ME"
        self.model = "general"
        self.base_url = "wss://spark-api.xf-yun.com/v1.1/chat"
        # ==================================================================

    def _build_websocket_url(self):
        """生成讯飞星火 WebSocket 连接 URL（含签名）- 官方标准方式"""
        ws_param = _Ws_Param(self.app_id, self.api_key, self.api_secret, self.base_url)
        return ws_param.create_url()

    def call_doubao_model(self, prompt, max_tokens=2048, temperature=0.7):
        """
        调用科大讯飞星火大模型API（WebSocket）
        :param prompt: 用户输入的文本
        :return: 统一格式响应，包含 choices 字段
        """
        global answer
        answer = ""  # 重置全局答案

        try:
            # 构建请求的消息格式（符合讯飞API要求）
            question = [{"role": "user", "content": prompt}]
            ws_url = self._build_websocket_url()

            # 创建WebSocket应用
            websocket.enableTrace(False)
            ws = websocket.WebSocketApp(
                ws_url,
                on_message=self._on_message,
                on_error=self._on_error,
                on_close=self._on_close
            )
            # 绑定自定义参数到WebSocket对象（兼容旧逻辑）
            ws.appid = self.app_id
            ws.question = question
            ws.domain = self.model
            ws.temperature = temperature
            ws.max_tokens = max_tokens

            # 打开连接并发送消息（修复：完整传参）
            def run():
                data = json.dumps(self._gen_params(question, temperature, max_tokens))
                ws.send(data)

            ws.on_open = lambda w: threading.Thread(target=run).start()
            ws.run_forever(sslopt={"cert_reqs": ssl.CERT_NONE})

            return {
                "choices": [{
                    "message": {
                        "role": "assistant",
                        "content": answer.strip()
                    }
                }]
            }

        except Exception as e:
            print(f"API调用异常: {str(e)}")
            return None

    def _gen_params(self, question, temperature, max_tokens):
        data = {
            "header": {
                "app_id": self.app_id,
                "uid": "interview_system"
            },
            "parameter": {
                "chat": {
                    "domain": self.model,
                    "temperature": temperature,
                    "max_tokens": max_tokens,
                    "top_k": 4,
                    "auditing": "default"
                }
            },
            "payload": {
                "message": {
                    "text": question
                }
            }
        }
        return data

    def _on_message(self, ws, message):
        """收到消息处理"""
        global answer, sid
        data = json.loads(message)
        code = data['header']['code']

        if code != 0:
            print(f'请求错误: {code}, {data}')
            ws.close()
            return

        sid = data["header"]["sid"]
        choices = data["payload"]["choices"]
        status = choices["status"]
        content = choices["text"][0]["content"]
        answer += content

        if status == 2:
            ws.close()

    def _on_error(self, ws, error):
        """错误处理"""
        print(f"### WebSocket错误: {error}")

    def _on_close(self, ws, one, two):
        """关闭处理"""
        pass

    # ===================== 以下业务函数完全保持原样，无需修改 =====================
    def analyze_resume(self, resume_content, job_position):
        """分析简历"""
        prompt = f"""
        请分析以下简历内容，并针对{job_position}岗位进行评估：

        {resume_content}

        请返回以下信息：
        1. 姓名、联系方式等基本信息
        2. 工作经验摘要
        3. 教育背景摘要
        4. 技能清单
        5. 项目经验摘要
        6. 与目标岗位的匹配点
        7. 可能的面试问题点
        """

        result = self.call_doubao_model(prompt)
        if result and "choices" in result:
            content = result["choices"][0]["message"]["content"]
            try:
                return json.loads(content)
            except json.JSONDecodeError:
                print("解析JSON失败，返回原始内容")
                return content
        return result

    def generate_interview_questions(self, resume_info, job_position, question_count=10):
        """生成面试问题"""
        prompt = f"""
        基于以下简历信息和目标岗位，生成{question_count}个面试问题：

        简历信息：{json.dumps(resume_info, ensure_ascii=False)}
        目标岗位：{job_position}

        请返回JSON格式，包含问题列表，每个问题包含：
        - text: 问题内容
        - type: 问题类型（如：行为题、技术题、情境题等）
        - difficulty: 难度等级（1-5）
        """

        result = self.call_doubao_model(prompt)
        if result and "choices" in result:
            content = result["choices"][0]["message"]["content"]
            try:
                return json.loads(content)
            except json.JSONDecodeError:
                print("解析JSON失败，返回原始内容")
                return content
        return result

    def analyze_answer(self, question, answer, resume_info, job_position):
        """分析回答"""
        prompt = f"""
        请分析以下面试回答：

        问题：{question}
        回答：{answer}
        简历信息：{json.dumps(resume_info, ensure_ascii=False)}
        目标岗位：{job_position}

        请从以下几个维度进行评分（0-100分）：
        - 专业知识分
        - 语言表达分
        - 逻辑思维分
        - 应变能力分

        并提供简要的评价和改进建议。
        """

        result = self.call_doubao_model(prompt)
        if result and "choices" in result:
            content = result["choices"][0]["message"]["content"]
            try:
                return json.loads(content)
            except json.JSONDecodeError:
                print("解析JSON失败，返回原始内容")
                return content
        return result

    def analyze_voice(self, text):
        """分析语音"""
        prompt = f"""
        请分析以下语音转文字内容，评估说话人的：
        1. 普通话水平
        2. 声音稳定性
        3. 回答清晰度
        4. 语言组织能力

        内容：{text}

        请以JSON格式返回评分（百分制）：
        {{
            "普通话水平": 分数,
            "声音稳定性": 分数,
            "回答清晰度": 分数,
            "语言组织": 分数
        }}
        """

        result = self.call_doubao_model(prompt)
        if result and "choices" in result:
            content = result["choices"][0]["message"]["content"]
            try:
                return json.loads(content)
            except json.JSONDecodeError:
                print("解析JSON失败，返回原始内容")
                return content
        return result

    def generate_interview_report(self, interview_data, facial_scores, voice_scores, professional_scores):
        """生成面试报告"""
        prompt = f"""
        请根据以下面试数据生成一份详细的面试报告：

        面试信息：{json.dumps(interview_data, ensure_ascii=False)}
        微表情分析：{json.dumps(facial_scores, ensure_ascii=False)}
        语音分析：{json.dumps(voice_scores, ensure_ascii=False)}
        专业知识评分：{json.dumps(professional_scores, ensure_ascii=False)}

        报告应包括：
        1. 面试概况
        2. 各项能力评估
        3. 优势与不足
        4. 改进建议
        5. 总体评价
        """

        result = self.call_doubao_model(prompt)
        if result and "choices" in result:
            return result["choices"][0]["message"]["content"]
        return result


class _Ws_Param(object):
    """内部使用：官方标准URL生成类"""
    def __init__(self, APPID, APIKey, APISecret, Spark_url):
        self.APPID = APPID
        self.APIKey = APIKey
        self.APISecret = APISecret
        self.host = urlparse(Spark_url).netloc
        self.path = urlparse(Spark_url).path
        self.Spark_url = Spark_url

    def create_url(self):
        now = datetime.now()
        date = format_date_time(mktime(now.timetuple()))

        signature_origin = f"host: {self.host}\ndate: {date}\nGET {self.path} HTTP/1.1"
        signature_sha = hmac.new(
            self.APISecret.encode('utf-8'),
            signature_origin.encode('utf-8'),
            digestmod=hashlib.sha256
        ).digest()
        signature_sha_base64 = base64.b64encode(signature_sha).decode('utf-8')

        authorization_origin = f'api_key="{self.APIKey}", algorithm="hmac-sha256", headers="host date request-line", signature="{signature_sha_base64}"'
        authorization = base64.b64encode(authorization_origin.encode('utf-8')).decode('utf-8')

        v = {"authorization": authorization, "date": date, "host": self.host}
        url = self.Spark_url + '?' + urlencode(v)
        return url