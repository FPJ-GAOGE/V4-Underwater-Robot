# V4-pro Jetson Orin NX 参考程序

中文 | [English](README.en.md)

本目录整理飞书《ORIN NX配置（ORIN NANO同样适用》的三个附件，对应 **V4-pro 的 Orin NX → ROBOMASTER A 板 UART3** 链路。脚本中的 `V5` 是历史文件名，不据此改写整机型号。

**`upstream/` 是保留原貌的历史参考，不是已经验收的推荐部署程序。** 没有在真实 Jetson、相机、手柄或 RM-A 上运行本次导入。原始脚本中的协议与参数应先和目标固件核对，再制作部署副本。V4 的树莓派 / RM-C / TCP 5000 客户端与这里的 UDP 手柄协议不兼容，不能直接互换。

## 来源与许可

来源文档：[ORIN NX配置（ORIN NANO同样适用](https://iwin-fins.feishu.cn/docx/S8I9dBMNmo2dIPxhWzkcyeQsnoP)。导入日期：2026-10-22。附件没有 Git 提交号或明确的开源许可证；本目录没有替原作者添加 MIT 等许可，公开发布授权需纳入项目整体的权属确认。

| 文件 | 用途 |
| --- | --- |
| `upstream/V5StreamerNX.py` | 接收 PC 的 UDP 手柄包，转换为串口控制帧；把串口收到的数据块原样转发到 PC UDP |
| `upstream/GStreamerTest.py` | 从 USB 相机采集 MJPEG/RAW，以 GStreamer 编码 H.264 后经 RTP/UDP 推流 |
| `upstream/StreamerStart.sh` | 原环境的一体化自启脚本，仅作历史参考 |
| `requirements.txt` | 串口桥接的 Python 依赖 `pyserial` |
| `deploy/*.service.example` | 新增的两个独立 systemd 模板；只提供进程管理方式，不证明源程序已适配当前实机 |

原自启脚本固定了原机器的 Conda 目录、Python 3.8、工作目录和环境名称，包含全员可写串口权限与按名称结束进程的做法。为保持证据，原文件未改写；新的部署模板不沿用这些做法。原资料中的设备账户和密码没有写入本说明或模板。

## 网络、串口和视频参数

| 参数 | 原始值 | 含义 |
| --- | --- | --- |
| `UDP_RECEIVE_PORT` | `12345` | 绑定所有网卡，接收 PC 手柄 UDP 数据报 |
| `UDP_SEND_IP` / `UDP_SEND_PORT` | `192.168.0.1:54321` | 下位机串口数据转发目的地 |
| `SERIAL_PORT` | `/dev/ttyTHS1` | Orin NX 载板 UART；具体引脚随载板版本核对 |
| `BAUDRATE` | `115200` | 串口波特率 |
| 串口格式 | `8N1` | 8 数据位、无校验、1 停止位；关闭 XON/XOFF、RTS/CTS、DSR/DTR 流控 |
| 串口读超时 | `0.1 s` | `serial.Serial(timeout=0.1)` |
| `VIDEO_DEVICE` | `/dev/video0` | USB 相机设备 |
| `CAMERA_MODE` | `mjpeg` | 默认 MJPEG 输入 |
| 视频尺寸 / 帧率 | `1280 × 480 @ 30 FPS` | 须与相机实际支持模式一致 |
| `VIDEO_BITRATE` | `4,000,000 bit/s` | NVENC 目标码率 |
| `FORCE_TARGET_IP` / `PC_VIDEO_PORT` | `192.168.0.1:5600` | RTP/H.264 视频目的地 |
| `RTP_PT` / `USE_NVENC` | `96` / `True` | RTP payload type；优先 NVIDIA 编码器 |

这些参数是源码内的常量；原脚本没有命令行参数解析，也不会自动读取环境变量覆盖它们。表中的私网 IP 为保留的部署示例值。

接线资料要求 Orin 对接 RM-A 的 UART3，TX 与 RX 交叉、GND 共地，并移除该串口线中的 5 V 供电线；应以具体载板手册和实际线序为准。这里的 UART3 / RM-A 不应照搬到 V4 的 RM-C 接线。

## UDP 手柄格式：41 字节

`JoystickState.from_bytes()` 使用 `struct.unpack('6f15B2b', data)`，主循环只接收长度恰好为 41 的数据报。

| UDP 偏移 | 类型 | 内容 |
| --- | --- | --- |
| 0–23 | `6f` | 六个 32 位浮点手柄轴值 |
| 24–38 | `15B` | 十五个无符号 8 位按钮值 |
| 39–40 | `2b` | 两个有符号 8 位方向帽值 |

**源码没有显式字节序前缀。** `6f15B2b` 和下述 `5f3B2b` 都依赖 Python `struct` 的原生字节序/对齐规则，不能把它们描述成架构无关的网络协议。本次检查环境为小端、计算长度分别为 41 / 25 字节；发送端必须与目标 Jetson 的实际布局一致。

## 下行串口格式：28 字节

实际发送序列是：

```text
0xAA | 25 字节数据（5f3B2b） | CRC16-MODBUS（2 字节，小端）
```

**没有长度字段，也没有 `0xBB` 帧尾。** CRC 的覆盖范围是 `0xAA + 25 字节数据`，初值 `0xFFFF`、反射多项式 `0xA001`，结果由 `struct.pack('<H', crc)` 写入。

| 数据区偏移（不含帧头） | 类型 | 字段 | 从手柄计算 |
| --- | --- | --- | --- |
| 0–3 | float32 | `yaw_rate` | `-axes[0]` |
| 4–7 | float32 | `forward_speed` | `-axes[3]` |
| 8–11 | float32 | `left_right_speed` | `-axes[2]` |
| 12–15 | float32 | `down_speed` | `clamp((axes[4]+1)/2, 0, 1)` |
| 16–19 | float32 | `up_speed` | `clamp((axes[5]+1)/2, 0, 1)` |
| 20 | uint8 | `start_button` | `buttons[6]` |
| 21 | uint8 | `cam_fill_light` | `buttons[0]` |
| 22 | uint8 | `frame_fill_light` | `buttons[1]` |
| 23 | int8 | `cam_yaw` | `hat[0]` |
| 24 | int8 | `cam_pitch` | `hat[1]` |

上行遥测没有在本脚本中重新组帧或校验：`ser.read(ser.in_waiting or 1)` 得到的每一块数据直接成为一个 UDP 数据报。串口读取块不保证等于完整固件消息，PC 接收端需要按实际协议重组。

已导入固件的对应实现位于 [`firmware/v4-pro/Services/V5Streamer/Streamer.hpp`](../../firmware/v4-pro/Services/V5Streamer/Streamer.hpp)，其帧结构与此附件一致。UART3 控制链路为 115200 8N1；固件中 IMU 使用的 UART7 / 921600 8N1 是另一条链路，不应混用。这里核验的是源代码对应关系，尚无实机联调结果。

## 部署准备与模板边界

PC端可使用原Orin文档中的GStreamer接收命令；先确保Jetson推流目标IP确为这台PC，端口均为5600，且PC已安装所需GStreamer插件：

```sh
gst-launch-1.0 -v udpsrc port=5600 caps="application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264, payload=(int)96" ! rtph264depay ! h264parse ! avdec_h264 ! videoconvert ! autovideosink
```

这是视频接收入口，不包含配套的Windows手柄发送程序；本次没有实际执行推流或接收。

Python 只额外需要 `pyserial`。GStreamer、NVIDIA 编解码插件、相机驱动与串口权限属于 Jetson / JetPack 的系统环境，不能通过 `pip install serial` 或单独安装这个 requirements 文件代替。

建议在 `/opt/iwin-v4/venv` 建立项目虚拟环境，把经核验的部署副本放到 `/opt/iwin-v4/runtime/`。例如，在目标 Jetson 已安装 Python venv 支持且目标目录已准备好后：

```bash
python3 -m venv /opt/iwin-v4/venv
/opt/iwin-v4/venv/bin/python -m pip install -r /opt/iwin-v4/source/software/v4-pro-jetson/requirements.txt
```

两个示例服务使用 `User=robot`、`Group=robot`，该用户/组需由部署者事先创建；这是示例账户名，不是原设备账户。串口服务加入 `dialout`，视频服务加入 `video`；若具体设备节点使用其他组，应调整模板到该组，并按设备的 udev 规则持久管理权限。服务直接使用虚拟环境的 Python，不依赖交互式 `conda activate`。

模板的 `ExecStart` 指向 `runtime/` 部署副本，**不会直接运行 `upstream/StreamerStart.sh`**。安装前先核对设备/固件协议并处理下面的已知限制，再将确认后的两个程序分别放入该路径。模板默认没有安装或启用，仓库整理过程中没有操作本机或实机 systemd。

准备完成后的部署操作示例：

```bash
sudo install -m 0644 deploy/iwin-v4-pro-control.service.example /etc/systemd/system/iwin-v4-pro-control.service
sudo install -m 0644 deploy/iwin-v4-pro-video.service.example /etc/systemd/system/iwin-v4-pro-video.service
sudo systemctl daemon-reload
sudo systemctl start iwin-v4-pro-control.service
sudo systemctl start iwin-v4-pro-video.service
journalctl -u iwin-v4-pro-control.service -u iwin-v4-pro-video.service -f
```

上述 `start` 是会打开串口、监听控制端口和启动视频的实际操作，不属于离线检查。本次没有执行。待实机验收后，才由部署者决定是否启用开机自启。两个服务独立管理，systemd 以服务 cgroup 清理进程，不使用全局名称匹配结束进程。

## 已知限制（保留源码，未修复）

1. `GStreamerTest.py` 的 `raw` 分支末尾没有 `!`，直接拼接编码器；离线构造的 token 序列确认缺少连接符，该模式不能作为已验证流水线。
2. `USE_NVENC=False` 时，前段仍输出 `video/x-raw(memory:NVMM)`，随后接 `videoconvert ! x264enc`；没有显式转回系统内存，CPU 回退链路存在 caps/内存域兼容问题，需要在目标 JetPack 验证和调整。
3. 源码注释声称的 profile / GOP 与实际参数不完全一致：代码没有设置 profile，`iframeinterval` 实际是 `int(VIDEO_FPS/2)`，默认 15。
4. `VideoStreamer.stop()` 捕获的是 Python 内置 `TimeoutError`，而 `Popen.wait(timeout=5)` 超时会抛 `subprocess.TimeoutExpired`；清理异常分支需在部署副本验证。systemd 模板的 cgroup 管理不能替代应用正确处理错误。
5. `V5StreamerNX.py` 在模块顶层打开串口、创建 socket 并进入循环；**不能直接 import 该模块做无副作用检查**。串口首次打开失败后没有重新打开机制。
6. UDP 收到 41 字节就按手柄包解码，没有来源身份校验、序号或命令过期拒绝；UDP 超时仅继续等待，不向 MCU 发送零输出。断联停机能力必须由目标固件及端到端验证确认，不能从这个桥接脚本推定。
7. 上行 `serial_receive_count` 没有在接收处递增，统计输出不能作为可靠收包计数。
8. GStreamer 子进程异常退出后，Python 主程序可能以成功状态退出；模板的 `Restart=on-failure` 不是视频健康检查。串口打开失败时进程可能仍在监听，也不能仅凭 systemd 的 active 状态判定链路正常。

本次完成静态语法与隔离的离线协议校验：只提取纯函数/类定义，以内存中的假串口接收结果，确认 UDP 长度 41、有效载荷长度 25、整帧长度 28、字段映射和 CRC 测试向量。没有导入串口脚本的顶层代码，没有打开串口/socket，也没有启动 GStreamer。结果不代表实机可运行。

检查环境为 Python 3.14.5、小端；两个 Python 附件静态编译通过，CRC16-MODBUS 的 `123456789` 测试向量为 `0x4B37`。仅离线构造 GStreamer 命令确认上述 raw 分支连接符和 CPU 分支 NVMM 限制，没有执行构造出的命令。

原始附件的 SHA-256：

| 文件 | SHA-256 |
| --- | --- |
| `V5StreamerNX.py` | `e6c97e906045df177d76d8b16685ee7cb6ad5dc434f04c072e52cb54ce6569b0` |
| `GStreamerTest.py` | `f00f7aed7b12b13e29c6668169819a32d81924ad64fccd49fb0bfee7a4da3692` |
| `StreamerStart.sh` | `bee9397ebb0cbe5d3f97b6cb76b76358e45ef7776723e313d4c86465845b168f` |
