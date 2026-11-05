import json
import requests


class VolcengineClientTextMixin:
    """DeepSeek 客户端 - 强制JSON输出、严格格式、自动解析"""

    def __init__(self):
        # ===================== DeepSeek 配置 =====================
        self.api_key = "CHANGE_ME"
        self.model = "deepseek-chat"
        self.base_url = "https://api.deepseek.com/v1/chat/completions"
        # =========================================================

    def call_doubao_model(self, prompt, max_tokens=3072, temperature=0.1):
        """
        强制返回JSON格式，temperature 极低保证格式稳定
        外部调用完全不变
        """
        # ===================== 核心：强制JSON指令 =====================
        force_json_prompt = f"""
你必须**只输出标准JSON**，不解释、不前言、不后缀、不废话、不换行多余内容。
严格按照要求输出JSON，格式必须合法可被python json.loads直接解析。

====================
用户需求：
{prompt}
====================

输出要求：只输出JSON，无任何其他文字。
"""
        headers = {
            "Authorization": f"Bearer {self.api_key}",
            "Content-Type": "application/json"
        }

        data = {
            "model": self.model,
            "messages": [{"role": "user", "content": force_json_prompt}],
            "temperature": temperature,  # 低温度保证格式稳定
            "max_tokens": max_tokens,
            "stream": False
        }

        try:
            response = requests.post(self.base_url, headers=headers, json=data, timeout=60)
            response.raise_for_status()
            result = response.json()
            content = result["choices"][0]["message"]["content"].strip()

            # 尝试清洗可能的包裹符号（```json ... ```）
            if content.startswith("```json"):
                content = content[7:-3].strip()
            elif content.startswith("```"):
                content = content[3:-3].strip()

            return {
                "choices": [{
                    "message": {
                        "role": "assistant",
                        "content": content
                    }
                }]
            }

        except Exception as e:
            print(f"DeepSeek API 异常: {str(e)}")
            return None

    # ===================== 以下全部强化：严格JSON格式 + 强制解析 =====================

    def analyze_resume(self, resume_content, job_position):
        prompt = f"""
你需要分析简历并**严格返回JSON对象**，字段如下：{{
    "基本信息": {{
        "姓名": "",
        "联系方式": "",
        "年龄": "",
        "性别": ""
    }},
    "工作经验摘要": "",
    "教育背景摘要": "",
    "技能清单": [],
    "项目经验摘要": "",
    "与目标岗位匹配点": [],
    "可能的面试问题点": []
}}

简历内容：
{resume_content}

目标岗位：{job_position}
"""
        result = self.call_doubao_model(prompt)
        if result and "choices" in result:
            try:
                return json.loads(result["choices"][0]["message"]["content"])
            except Exception as e:
                print("analyze_resume JSON解析失败:", e)
                return result["choices"][0]["message"]["content"]
        return None

    def generate_interview_questions(self, resume_info, job_position, question_count=10):
        prompt = f"""
生成{question_count}个面试题，**只返回如下结构的JSON**：{{
    "questions": [
        {{
            "text": "问题内容",
            "type": "技术题/行为题/情境题/项目题",
            "difficulty": 1~5
        }}
    ]
}}

简历信息：{json.dumps(resume_info, ensure_ascii=False)}
目标岗位：{job_position}
"""
        result = self.call_doubao_model(prompt)
        if result and "choices" in result:
            try:
                return json.loads(result["choices"][0]["message"]["content"])
            except Exception as e:
                print("generate_interview_questions JSON解析失败:", e)
                return result["choices"][0]["message"]["content"]
        return None

    def analyze_answer(self, question, answer, resume_info, job_position):
        prompt = f"""
对面试回答评分，**必须返回如下JSON结构**，分数0-100：{{
    "专业知识分": 0~100,
    "语言表达分": 0~100,
    "逻辑思维分": 0~100,
    "应变能力分": 0~100,
    "评价": "简要评价",
    "改进建议": "具体建议"
}}

问题：{question}
回答：{answer}
简历：{json.dumps(resume_info, ensure_ascii=False)}
岗位：{job_position}
"""
        result = self.call_doubao_model(prompt)
        if result and "choices" in result:
            try:
                return json.loads(result["choices"][0]["message"]["content"])
            except Exception as e:
                print("analyze_answer JSON解析失败:", e)
                return result["choices"][0]["message"]["content"]
        return None

    def analyze_voice(self, text):
        prompt = f"""
分析语音转写文本，**严格返回如下JSON**，均为0-100整数：{{
    "普通话水平": 0~100,
    "声音稳定性": 0~100,
    "回答清晰度": 0~100,
    "语言组织": 0~100
}}

内容：{text}
"""
        result = self.call_doubao_model(prompt)
        if result and "choices" in result:
            try:
                return json.loads(result["choices"][0]["message"]["content"])
            except Exception as e:
                print("analyze_voice JSON解析失败:", e)
                return result["choices"][0]["message"]["content"]
        return None

    def generate_interview_report(self, interview_data, facial_scores, voice_scores, professional_scores):
        prompt = f"""
生成面试报告，**返回JSON结构**：{{
    "面试概况": "",
    "各项能力评估": {{}},
    "优势": [],
    "不足": [],
    "改进建议": [],
    "总体评价": ""
}}

数据：
面试信息：{json.dumps(interview_data, ensure_ascii=False)}
微表情：{json.dumps(facial_scores, ensure_ascii=False)}
语音：{json.dumps(voice_scores, ensure_ascii=False)}
专业：{json.dumps(professional_scores, ensure_ascii=False)}
"""
        result = self.call_doubao_model(prompt, max_tokens=4096)
        if result and "choices" in result:
            raw = result["choices"][0]["message"]["content"]
            try:
                return json.loads(raw)
            except Exception as e:
                print(f"generate_interview_report JSON解析失败: {e}")
                print(f"原始内容(前500字)：{raw[:500]}")
                return raw   # 返回原始字符串而不是 None，前端可直接显示
        print("generate_interview_report：call_doubao_model 返回 None")
        return None
