# 历史通信示例导入记录

中文 | [English](IMPORT.en.md)

本目录仅用于保留 2025 年 V4 研发资料中的早期 TCP 图像通信示例，**不是当前 V4 树莓派 `joystick_control` 服务，也不是 V4-pro 的 UDP / UART 桥接实现**。

来源为飞书[上位机和树莓派通信](https://iwin-fins.feishu.cn/wiki/YMpxw929tiODo1kBmVRcYpy4noh)相关资料附件 `wirelesscommunicator.zip`。导入日期：2026-10-22。附件未包含 Git 提交号或明确许可；不能据此假定 MIT 等开源授权。公开发布时应随项目确认原作者授权。

仅提取归档内的 7 个 `.py` 文件，保留原文件名和字节；不导入 `.idea/` 工程配置、设备照片/测试照片、日志或运行环境。`main_sever.py` 的拼写来自原文件，未擅自更名。代码里没有发现密码、访问令牌或私钥；私网 IP 作为原始历史配置保留。

| 文件 | 历史用途 |
| --- | --- |
| `server.py`、`main_sever.py` | 树莓派侧 TCP 图像服务，打开相机 0 |
| `client.py`、`main_client.py` | PC 侧图像接收和显示 |
| `server_test.py`、`client_test.py` | 单张图片 TCP 传输尝试，分别使用不同的历史 IP |
| `camera test.py` | 本地相机 1 的 OpenCV 预览测试 |

主要传输端口为 **TCP 12345**。当前 V4 PC 手柄客户端使用 **TCP 5000**，V4-pro 使用 **UDP 12345**；端口数字相同不代表传输层或协议相同。这里没有 MCU 串口、手柄转发或当前 `joystick_control` systemd 服务的实现。

历史依赖为 Python 标准库及 OpenCV（导入名 `cv2`，通常对应 `opencv-python`）。原附件没有 Python/OpenCV 版本约束，未提供依赖锁，也未验证现代环境兼容性。

## 已知不完整处

- `server.py` 虽生成 JPEG 编码及 `pickle.dumps(buffer)`，实际却发送 `len(image)` 和 `image`，没有发送已经生成的 `data`。它与客户端期待的“4 字节网络序长度 + pickle 载荷”不匹配，不能作为可运行的图像协议示范。
- `client.py` 接收完载荷后又读取一次 socket，并且局部缓冲不能持续保留下一帧，存在阻塞/丢帧问题。连接提前关闭时也没有完整的 EOF 处理。
- `client.py` 使用 `pickle.loads` 反序列化网络输入，这会执行恶意 pickle 中的代码；不要对不可信网络数据运行该历史实现。
- `server_test.py` 依赖 `image.jpg`，该文件未随本次代码导入；照片不属于本次源码交付内容。
- 多个脚本在模块顶层就打开摄像头、创建/连接 socket 或进入循环；不要通过直接 import 来做静态检查。

本次不修复这些历史算法或协议，仅进行 AST / compile 静态语法检查并验证 7 个文件与 ZIP 中的原字节一致。没有启动网络、相机或图像程序，也没有将其接入当前 V4/V4-pro 的运行流程。
