"""
AI Proxy Server — FastAPI 版
通过后端转发请求，保护 DeepSeek API Key。

用户手机 → 本服务 → DeepSeek API
多 worker 稳定运行，不易断连。
"""

import os
import json
from contextlib import asynccontextmanager
from typing import List, Dict, Any

import httpx
from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import JSONResponse

# ============ 配置区 ============
DEEPSEEK_API_KEY = "CHANGE_ME"
DEEPSEEK_API_URL = "https://api.deepseek.com/chat/completions"
DEEPSEEK_MODEL = "deepseek-chat"
DEEPSEEK_TIMEOUT = 120.0  # 秒
# ================================

# 全局异步 HTTP 客户端（连接池复用）
# 与 DeepSeek 的连接会被复用，减少建连开销
_http_client: httpx.AsyncClient | None = None


def get_http_client() -> httpx.AsyncClient:
    global _http_client
    if _http_client is None:
        _http_client = httpx.AsyncClient(
            timeout=httpx.Timeout(DEEPSEEK_TIMEOUT),
            limits=httpx.Limits(
                max_connections=100,
                max_keepalive_connections=20,
                keepalive_expiry=30.0,
            ),
        )
    return _http_client


@asynccontextmanager
async def lifespan(app: FastAPI):
    """启动时创建 HTTP client，关闭时优雅释放"""
    yield
    if _http_client:
        await _http_client.aclose()


app = FastAPI(
    title="AI Proxy",
    version="2.0",
    lifespan=lifespan,
)

# ─── CORS：允许所有来源（移动端必须）────────────────────────────
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"],
)


# ─── 路由 ───────────────────────────────────────────────────

@app.post("/chat")
async def chat(body: Dict[str, Any]) -> JSONResponse:
    """
    接收客户端消息，转发给 DeepSeek。

    请求体:
        {
          "messages": [
            {"role": "user", "content": "..."},
            {"role": "assistant", "content": "..."},
            {"role": "system", "content": "..."}
          ]
        }
    """
    messages = body.get("messages", [])
    if not messages:
        raise HTTPException(status_code=400, detail="messages 不能为空")

    payload = {
        "model": DEEPSEEK_MODEL,
        "messages": messages,
        "stream": False,
    }
    headers = {
        "Content-Type": "application/json",
        "Authorization": f"Bearer {DEEPSEEK_API_KEY}",
    }

    try:
        client = get_http_client()
        resp = await client.post(
            DEEPSEEK_API_URL,
            json=payload,
            headers=headers,
        )
        resp.raise_for_status()

        # 直接透传 DeepSeek 的原始响应，保留 choices 等所有字段
        return JSONResponse(content=resp.json(), status_code=200)

    except httpx.TimeoutException:
        raise HTTPException(status_code=504, detail="请求超时，请稍后重试")

    except httpx.HTTPStatusError as e:
        # DeepSeek 返回了错误 HTTP 状态码（如 401、429、500）
        # 尝试把错误信息传回客户端
        try:
            err_body = e.response.json()
            error_msg = err_body.get("error", {}).get("message", str(e))
        except Exception:
            error_msg = str(e)
        raise HTTPException(status_code=e.response.status_code, detail=error_msg)

    except httpx.RequestError as e:
        raise HTTPException(status_code=502, detail=f"无法连接 DeepSeek: {e}")


@app.get("/health")
async def health():
    """健康检查"""
    return {"status": "ok"}


# ─── 启动 ───────────────────────────────────────────────────
if __name__ == "__main__":
    import uvicorn

    port = int(os.environ.get("PORT", 5000))
    print(f"[AI Proxy] 启动中，端口 {port}")
    # workers=4：4个独立进程，不共享 GIL，并发能力大幅提升
    uvicorn.run(
        "server:app",
        host="0.0.0.0",
        port=port,
        workers=4,           # 关键：多进程，不易崩溃
        loop="uvloop",        # 高速事件循环
        access_log=False,     # 减少日志开销
    )
