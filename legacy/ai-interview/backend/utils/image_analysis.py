import json
import hashlib
import hmac
import base64
import threading
import ssl
import re
from datetime import datetime
from time import mktime
from urllib.parse import urlparse, urlencode
from wsgiref.handlers import format_date_time
import websocket

class VolcengineClientImageMixin:
    """图像分析混合类（严格遵循代码A的连接方式和消息格式）"""

    def __init__(self):
        self.app_id = "CHANGE_ME"
        self.api_key = "CHANGE_ME"
        self.api_secret = "CHANGE_ME"
        self.base_url = "wss://spark-api.cn-huabei-1.xf-yun.com/v2.1/image"
        # 实例变量：每次调用独立，避免多请求并发时全局变量被覆盖导致堆损坏
        self._answer = ""
        self._sid = ''

    # ===================== 连接方法（完全参照代码A的Ws_Param） =====================
    def _create_ws_url(self):
        class _WsParam:
            def __init__(self, appid, api_key, api_secret, url):
                self.APPID = appid
                self.APIKey = api_key
                self.APISecret = api_secret
                self.host = urlparse(url).netloc
                self.path = urlparse(url).path
                self.url = url

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
                return self.url + '?' + urlencode(v)

        ws_param = _WsParam(self.app_id, self.api_key, self.api_secret, self.base_url)
        return ws_param.create_url()

    def _call_xf_model(self, messages, max_tokens=2048, temperature=0.7):
        self._answer = ""   # 每次调用重置自己的实例变量

        try:
            ws_url = self._create_ws_url()
            websocket.enableTrace(False)
            # 用闭包捕获 self 引用，确保回调操作的是当前实例
            client = self
            ws = websocket.WebSocketApp(
                ws_url,
                on_message=client._on_message,
                on_error=client._on_error,
                on_close=client._on_close
            )
            ws.appid = self.app_id
            ws.messages = messages
            ws.temperature = temperature
            ws.max_tokens = max_tokens

            def run():
                data = json.dumps(client._gen_params(ws.messages, ws.temperature, ws.max_tokens))
                ws.send(data)

            ws.on_open = lambda w: threading.Thread(target=run).start()
            ws.run_forever(sslopt={"cert_reqs": ssl.CERT_NONE})

            return {
                "choices": [{
                    "message": {
                        "role": "assistant",
                        "content": self._answer.strip()
                    }
                }]
            }

        except Exception as e:
            print(f"图像分析API调用异常: {str(e)}")
            return None

    def _gen_params(self, messages, temperature, max_tokens):
        data = {
            "header": {
                "app_id": self.app_id,
                "uid": "interview_system"
            },
            "parameter": {
                "chat": {
                    "domain": "imagev3",
                    "temperature": temperature,
                    "max_tokens": max_tokens,
                    "top_k": 4,
                    "auditing": "default"
                }
            },
            "payload": {
                "message": {
                    "text": messages
                }
            }
        }
        return data

    def _on_message(self, ws, message):
        data = json.loads(message)
        code = data['header']['code']
        if code != 0:
            print(f'图像请求错误: {code}, {data}')
            ws.close()
            return
        self._sid = data["header"]["sid"]
        choices = data["payload"]["choices"]
        status = choices["status"]
        content = choices["text"][0]["content"]
        self._answer += content
        if status == 2:
            ws.close()

    def _on_error(self, ws, error):
        print(f"### WebSocket 错误: {error}")

    def _on_close(self, ws, one, two):
        pass

    # ===================== 业务函数 =====================
    def analyze_facial_expression(self, image_data):
        """
        分析面部表情（严格返回JSON格式）
        :param image_data: 图片的 base64 编码数据（可带 data:image/xxx;base64, 前缀）
        :return: 包含评分信息的字典，格式：{"紧张程度": int, "着装整洁度": int, "眼神接触": int, "面部表情": int}
        """
        # 提取纯 base64
        if image_data.startswith('data:image'):
            img_base64 = image_data.split(',', 1)[1]
        else:
            img_base64 = image_data

        # 强化提示词：要求只输出纯JSON，不要有任何额外文字
        instruction = (
            "请分析以下面部表情图片数据，评估候选人的紧张程度、眼神接触、面部表情自然度等。\n"
            "**严格要求**：只输出一个纯JSON对象，不要有任何额外文字、解释、Markdown格式或注释。\n"
            "JSON格式必须严格如下：\n"
            "{\n"
            "    \"紧张程度\": 分数（整数，0-100）,\n"
            "    \"着装整洁度\": 分数（整数，0-100）,\n"
            "    \"眼神接触\": 分数（整数，0-100）,\n"
            "    \"面部表情\": 分数（整数，0-100）\n"
            "}\n"
            "只输出JSON，不要输出其他任何内容。"
        )

        # 严格按照代码A的消息顺序：先图片消息，后文本消息
        messages = [
            {"role": "user", "content": img_base64, "content_type": "image"},
            {"role": "user", "content": instruction}
        ]

        result = self._call_xf_model(messages)

        # 默认评分（当解析失败时使用）
        default_scores = {
            "紧张程度": 50,
            "着装整洁度": 70,
            "眼神接触": 50,
            "面部表情": 60
        }

        if result and "choices" in result:
            content = result["choices"][0]["message"]["content"]
            # 尝试多种方式提取JSON
            try:
                # 1. 直接解析
                return json.loads(content)
            except json.JSONDecodeError:
                # 2. 使用正则提取第一个完整的JSON对象
                json_pattern = r'\{[^{}]*"紧张程度"\s*:\s*\d+[^{}]*\}'
                # 更通用的提取：匹配平衡的大括号
                try:
                    # 找到第一个 { 和对应的 }
                    start = content.find('{')
                    if start != -1:
                        brace_count = 0
                        for i in range(start, len(content)):
                            if content[i] == '{':
                                brace_count += 1
                            elif content[i] == '}':
                                brace_count -= 1
                                if brace_count == 0:
                                    json_str = content[start:i+1]
                                    # 尝试修复可能的格式问题（如缺少引号）
                                    json_str = re.sub(r'(\w+):', r'"\1":', json_str)  # 给key加引号
                                    return json.loads(json_str)
                except:
                    pass

                # 3. 如果都失败，打印警告并返回默认评分
                print(f"警告：AI返回内容无法解析为JSON，原始内容：{content[:200]}...")
                return default_scores

        # 如果调用失败，返回默认评分
        print("警告：AI调用失败，返回默认评分")
        return default_scores