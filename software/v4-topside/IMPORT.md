# V4 PC 上位机导入记录

## 来源与适用范围

- 上游仓库：<https://gitee.com/superkinkin/rov-v4-wired-joystick>
- 上游分支：`master`
- 固定提交：`a65bc514808d2d8de929f05d2f42f31ed5bc848c`
- 导入日期：2026-10-22
- 适用链路：V4 的 PC 手柄客户端 → 以太网 TCP → 树莓派 → RM-C 下位机。
- 该导入不证明与指定的 `V4-ly` 固件已完成配套联调，也不适用于直接驱动 V4-pro 的 Orin NX / RM-A 链路。

## 许可状态

本次固定提交没有 `LICENSE`、`COPYING`、SPDX 标识或其他明确的代码许可声明。原始 `README.md` 也未授权以 MIT、Apache-2.0 等条款重新分发。

这里依据项目所有者要求收录到私有开源准备仓库，保留原始来源与提交；尚未为这些上游文件追加新许可。公开发布前，应由项目权利人确认这部分代码的授权和最终许可。仓库其他部分的许可不能自动覆盖本目录上游代码。

## 文件对应关系

| 文件 | 运行位置与用途 |
| --- | --- |
| `pc_client.py` | PC：Pygame 手柄控制界面，收取下位机返回的行文本并写入 `recv_log.txt`；包含舵机开始/停止指令 |
| `pc_client_plot.py` | PC：带实时曲线的控制界面，按数字键选择曲线；仍然会发送控制指令，不能当作只读遥测工具 |
| `test.py` | PC：交互式 TCP 文本发送调试程序；不是自动测试，也不是树莓派服务端 |
| `stream.sdp` | PC：FFplay 接收 RTP/H.264 的描述文件 |
| `README.md` | 上游使用说明，原样保留 |
| `requirements.txt` | 本次新增的 PC Python 依赖清单 |

**树莓派程序没有包含在该上游提交中。** 飞书《V4使用说明》提到树莓派主要代码位于 `joystick_control`，已配置 systemd 自启动，但该资料没有给出具体服务名、启动入口或脚本内容。不能仅克隆本目录就重建树莓派服务。上游也没有环境 YAML、固件、摄像头推流脚本或 systemd unit 文件。

已检查的关联资料还包含 TCP 12345 图片传输的历史示例；它不能充当当前 TCP 5000 的 `joystick_control` 服务。当前服务端源码未出现在本次已获取的引用仓库或附件中，本目录没有自行补写替代服务端。

## PC 最小安装与启动

以下命令在本目录执行。上游未提供 Python/Pygame 版本约束；建议在目标 PC 的独立虚拟环境中安装，完成手柄兼容性确认后记录版本。

```powershell
python -m venv .venv
.\.venv\Scripts\python.exe -m pip install -r requirements.txt
```

客户端直接在源文件中配置地址，三个程序的默认值如下。它们是原始设备网络示例，不包含登录凭证。

| 配置 | 文件 | 默认值 |
| --- | --- | --- |
| 树莓派 IP | `pc_client.py`、`pc_client_plot.py` 的 `PI_IP`；`test.py` 的 `RASPBERRY_IP` | `192.168.138.2` |
| 控制连接 | 对应的 `PI_PORT` / `RASPBERRY_PORT` | TCP `5000` |
| PC 地址示例 | 飞书《V4使用说明》 | `192.168.138.10`，须与实际设备同网段 |

确认设备地址、树莓派服务、固件协议和手柄轴映射后，再运行其中一个客户端：

```powershell
.\.venv\Scripts\python.exe pc_client.py
# 或使用带曲线的控制客户端；两者无需同时启动
.\.venv\Scripts\python.exe pc_client_plot.py
```

两个客户端启动都会创建 GUI 并尝试连接设备。不要通过 `import pc_client` 来做无副作用的检查，因为 GUI 初始化位于模块顶层。`test.py` 的手动文本发送不能替代下述二进制协议。

### 视频接收

使用上游默认的 GStreamer 方案，需另行安装 GStreamer 及相应 H.264 插件，确认树莓派推流目标为本机 UDP 5600：

```text
gst-launch-1.0 -v udpsrc port=5600 caps="application/x-rtp, media=video, encoding-name=H264, payload=96, clock-rate=90000" ! rtpjitterbuffer latency=10 drop-on-latency=true ! rtph264depay ! avdec_h264 ! videoconvert ! autovideosink sync=false
```

上游另提供 FFplay 方案，`stream.sdp` 的接收地址是 `192.168.138.10`、RTP 端口是 **5004**，与上述 GStreamer 的 **5600** 不同。须按实际推流方案修改，不能将两个默认端口视为一致：

```text
ffplay -fflags nobuffer -flags low_delay -sync video -framedrop -probesize 32 -analyzeduration 0 -protocol_whitelist file,udp,rtp,tcp,https,crypto -i stream.sdp
```

## 与固件配套时必须核对的接口

本目录只实现 TCP 客户端，**没有串口设备名、波特率、数据位、校验位或停止位的配置**。不能从 TCP 端口 5000 推导串口参数。树莓派转发程序与 `V4-ly` 固件的 UART 初始化必须一起核对；飞书 V4 接线资料涉及 RM-C 的 UART6，但接线位置仍不足以证明代码中的波特率和串口选择。

PC 的 `send_pkt` 直接将以下字节写入 TCP 流，不添加长度、校验或帧尾。树莓派若负责组帧，须由其实际源码确认；TCP 单次 `recv` 不保证对应一个完整命令。

| 功能 | PC 字节序列 | 实际调用 |
| --- | --- | --- |
| 悬浮开关 | `02 10` | 按钮 7；本地 `isHovering` 状态随之翻转 |
| 偏航闭环开关 | `02 20` | 按钮 0 |
| 平面推力 | `02 30` + `int8 x` + `int8 y` | 左摇杆；约为 -99～99 |
| 偏航角速度 | `02 23` + 小端 `int16` | 右摇杆水平 |
| 深度力输入 | `02 02` + 小端 `int16` | 右摇杆垂直 |
| 舵机开始 / 停止 | `03 00` / `03 01` | 仅 `pc_client.py` 按钮 1 / 2 |

需要逐项核对的现有行为：

1. `pc_client.py` 在悬浮状态下仅当指令值变化时发送平面、偏航、深度控制；`pc_client_plot.py` 则在每次主循环发送，循环目标是 30 FPS。两者发送频率不同，可能影响具有通信超时策略的固件。
2. 普通客户端的心跳发送代码处于注释状态。不能因存在 `HEARTBEAT_INTERVAL` 常量就认定存在持续心跳。
3. 程序退出只关闭 socket，没有显式发送停机帧；断线处理和停机策略依赖树莓派及固件。本次未改变上游控制逻辑。
4. GUI 中的深度目标和十字键舵机角度不等同已发送的有效控制量。普通客户端的 `MOT:` 舵机角度发送处于注释状态，绘图客户端也未发送该角度。
5. `pc_client_plot.py` 只能解析按换行分隔的文本数字数据；实际回传格式必须匹配。曲线客户端与普通客户端不能用未核实的二进制遥测格式替换。
6. 手柄轴编号固定为 0、1、2、3。飞书使用说明明确提示不同系统的映射可能不同；必须先核验输入值和对应方向。

以上为固定提交的静态代码事实，不构成已通过实机验证的兼容性结论。

## 导入净化与验证范围

- 原始 Git 对象导出：`pc_client.py`、`pc_client_plot.py`、`test.py`、`stream.sdp`、`README.md`。通过固定提交的 `git -c core.autocrlf=false show <commit>:<path>` 读取原字节，消除 Windows checkout 自动转换 CRLF 的影响；未改变程序内容。
- 排除：上游 `.git/` 历史、空 `recv_log.txt`。未导入可执行文件、虚拟环境、运行日志或凭证。
- 上游三个 Python 文件和视频配置中未发现硬编码密码、访问令牌或私钥；没有实际密码需要替换。设备示例 IP 保留以维持原始配置事实，部署时按现场值修改。
- 新增：本导入记录、仅含 `pygame` 的依赖清单，以及适用于 Python 的本地 `.gitignore`。上游没有锁定依赖版本，因此这里没有声称已有可复现的版本锁。
- 本次仅执行离线静态语法解析、依赖导入清单检查、与上游 SHA-256 内容一致性检查；没有启动客户端、连接实机或发送控制命令。
- 检查环境为 Python 3.14.5，三个 Python 文件均通过 AST 解析和静态编译；五个来源文件均按 `blob <字节数>\0<内容>` 计算 Git blob ID，并与固定上游提交的对象 ID 一致，另外记录 SHA-256。检查环境没有安装 `pygame`，未执行 GUI 或依赖安装验证。
- Python 静态语法通过只表示文件可解析，不代表 GUI、游戏手柄、网络、视频或 `V4-ly` 固件链路已验证。
