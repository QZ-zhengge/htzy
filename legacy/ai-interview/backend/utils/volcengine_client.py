from .text_analysis import VolcengineClientTextMixin
from .image_analysis import VolcengineClientImageMixin


class VolcengineClient(VolcengineClientTextMixin, VolcengineClientImageMixin):
    """完整的Volcengine客户端，组合文本和图像分析功能"""
    pass


volcengine_client = VolcengineClient()