# -*- coding: utf-8 -*-
import os
import socket
import time
import pygame
import struct
import threading
from collections import deque, defaultdict

PI_IP = "192.168.138.2"
PI_PORT = 5000
DIRECTION_THRESHOLD = 0.1
DEPTH_THRESHOLD = 0.2
ROTATE_THRESHOLD = 0.2
HEARTBEAT_INTERVAL = 0.5
FPS = 30

# 深度
DEPTH_MIN = 0
DEPTH_MAX = 1000
DEPTH_STEP = 10
DEPTH_REPEAT_INTERVAL = 0.1

# 舵机
PWM_MIN, PWM_MAX = 500, 2500
SERVO_STEP = 20
SERVO_REPEAT_INTERVAL = 0.10
aaaa = 1500
bbbb = 1500
cccc = 1500
dddd = 1500
target_depth = 300

MAX_SPEED = 99  # 最大速度值
FRAME_HEAD = 0x02
CMD_SET_DEPTH_VALUE = 0x00  # [int16 depth_x10]
CMD_SET_DEPTH_INCRE = 0x01  # [int16 depth_x10]
CMD_SET_DEPTH_FORCE = 0x02  # [int16 force]
CMD_TOGGLE_FLOAT = 0x10  #
CMD_TOGGLE_YAW_LOOP = 0x20  #
CMD_SET_YAW_VALUE = 0x21  # [int16 deg]
CMD_INC_YAW_INCRE = 0x22  # [int16 deg]
CMD_SET_YAW_RATE = 0x23  # [int16 deg_per_s]，0=停转并锁角
CMD_SET_PLANAR_VEL = 0x30  # [int8 x, int8 y]，-99..99
isHovering = False

pygame.init()
screen = pygame.display.set_mode((900, 520))  # 增大画布，添加实时曲线区域
pygame.display.set_caption("Joystick TCP Controller + Realtime Plot")
clock = pygame.time.Clock()
font_small = pygame.font.Font(None, 20)
font_norm = pygame.font.Font(None, 24)


def draw_text(x, y, text, font=font_small, color=(0, 0, 0)):
    surface = font.render(text, True, color)
    screen.blit(surface, (x, y))
    return y + surface.get_height() + 2


def pwm_to_deg(pwm, pwm_min=PWM_MIN, pwm_max=PWM_MAX, full_deg=270.0):
    val = (pwm - pwm_min) / (pwm_max - pwm_min)
    val = max(0.0, min(1.0, val))
    return round(val * full_deg, 1)

# -------------------------
#  原 recv_log 相关：已注释掉（不再保存至 recv_log）
# -------------------------
# recv_log = deque(maxlen=20)
rx_thread = None
rx_stop = threading.Event()
log_file_path = "recv_log.txt"

# 已禁用：写入 recv_log 与文件
# def log_recv(msg):
#     ts = time.strftime("%H:%M:%S")
#     line = f"{ts}  {msg}"
#     recv_log.appendleft(line)
#     try:
#         with open(log_file_path, "a", encoding="utf-8") as f:
#             f.write(line + "\n")
#     except Exception as e:
#         print(f"[FILE] 写入日志失败: {e}")

# -------------------------
#   实时数据接收与绘图缓存
# -------------------------
# 缓冲每个通道最近 N 个数值，用于画图
PLOT_HISTORY = 600  # 约 20 秒历史（30 FPS）
channel_buffers = defaultdict(lambda: deque(maxlen=PLOT_HISTORY))
# 保存每个通道最近一个数值，便于 UI 展示
channel_latest = {}
# 若行内没有 key，仅单值，则落到 default 通道
DEFAULT_CHANNEL_NAME = "value"
# 控制最多同时绘制多少条通道曲线
MAX_CHANNELS_TO_DRAW = 3
# 当前选择要显示的通道集合（按键切换）
selected_channels = []  # 空则按照出现顺序自动选择前 MAX_CHANNELS_TO_DRAW 个

# 简单的颜色表（pygame 使用 RGB）
COLOR_TABLE = [
    (0, 0, 0), (200, 0, 0), (0, 120, 0), (0, 0, 200),
    (200, 120, 0), (120, 0, 200), (0, 150, 150), (120, 120, 120),
]


def parse_numeric(s):
    try:
        if ":" in s or "=" in s:
            # 形如 key:val 或 key=val
            if ":" in s:
                k, v = s.split(":", 1)
            else:
                k, v = s.split("=", 1)
            k = k.strip()
            v = v.strip()
            val = float(v)
            return k, val
        # 仅有一个数字
        return DEFAULT_CHANNEL_NAME, float(s.strip())
    except Exception:
        return None


def ingest_line(text: str):
    """解析一行文本，将其中的 key:value（或 key=value）或裸数值存入 channel_buffers。
    支持以逗号、空格分隔的多组数据，例如：
        "depth: 2.31, yaw=5.0 roll=0.2"
        "123.4"  -> 归入 'value' 通道
    """
    # 尝试用逗号拆分，再用空白拆分，尽量多提取
    parts = []
    for chunk in text.replace("\t", " ").split(","):
        parts.extend(chunk.strip().split())

    found = False
    for token in parts:
        parsed = parse_numeric(token)
        if parsed is None:
            continue
        k, val = parsed
        channel_buffers[k].append(val)
        channel_latest[k] = val
        found = True

    # 如果一行一点可解析数据都没有，忽略
    return found


def start_rx_thread():
    global rx_thread, rx_stop
    rx_stop.clear()
    rx_thread = threading.Thread(target=recv_worker, daemon=True)
    rx_thread.start()


def recv_worker():
    """后台线程：接收下位机发来的行文本，解析并写入绘图缓存"""
    buf = b""
    while not rx_stop.is_set():
        try:
            data = sock.recv(1024)
            if not data:
                # 连接丢失，尝试重连
                print("# 连接丢失，正在重连...")
                connect_to_pi()
                continue
            buf += data
            while b"\n" in buf:
                line, buf = buf.split(b"\n", 1)
                text = line.decode(errors="ignore").strip("\r")
                if text:
                    # 不再写入 recv_log / 文件，只解析入绘图缓存
                    ingest_line(text)
        except Exception as e:
            print(f"# 接收错误: {e}")
            time.sleep(1)


sock = None
send_log = deque(maxlen=12)


def connect_to_pi():
    global sock
    while True:
        try:
            if sock:
                try:
                    sock.close()
                except:
                    pass
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.connect((PI_IP, PI_PORT))
            sock = s
            print(f"[NET] Connected to {PI_IP}:{PI_PORT}")
            log_send(f"# Connected {PI_IP}:{PI_PORT}")
            start_rx_thread()
            return
        except Exception as e:
            print(f"[NET] Connect failed: {e}, retrying...")
            time.sleep(3)


def log_send(msg):
    ts = time.strftime("%H:%M:%S")
    send_log.appendleft(f"{ts}  {msg}")


def send_tcp(msg):
    global sock
    data = (msg + "\n").encode()
    try:
        sock.sendall(data)
        log_send(msg)
    except Exception as e:
        print(f"[NET] Send failed: {e}")
        connect_to_pi()


def send_pkt(code, payload=b""):
    """发送一帧二进制：head(1) + code(1) + payload(n)"""
    global sock
    frame = bytes([FRAME_HEAD, code]) + payload
    try:
        sock.sendall(frame)
        log_send(f"TX data={frame.hex(' ')}")
    except Exception as e:
        print(f"[NET] Send failed: {e}")
        connect_to_pi()


def i16le(val):
    return struct.pack("<h", int(val))


def i8(val):
    # 截断到 int8 范围
    v = int(max(-128, min(127, int(val))))
    return struct.pack("b", v)


def calculate_speed(axis_value, threshold):
    """根据摇杆轴值计算速度（0-MAX_SPEED）"""
    speed = 0
    if not -threshold < axis_value < threshold:
        if axis_value > 0:
            speed = int((axis_value - threshold) * MAX_SPEED / (1 - threshold))
        else:
            speed = -int((-axis_value - threshold) * MAX_SPEED / (1 - threshold))
    return speed


# -------------------------
#     绘图辅助（在 pygame 中画折线图）
# -------------------------

def draw_axes(rect, x_ticks=6, y_ticks=4):
    x, y, w, h = rect
    # 边框
    pygame.draw.rect(screen, (30, 30, 30), rect, 1)
    # 网格
    for i in range(1, x_ticks):
        xx = x + int(w * i / x_ticks)
        pygame.draw.line(screen, (220, 220, 220), (xx, y), (xx, y + h), 1)
    for j in range(1, y_ticks):
        yy = y + int(h * j / y_ticks)
        pygame.draw.line(screen, (230, 230, 230), (x, yy), (x + w, yy), 1)


def draw_series(rect, name, data, color):
    if len(data) < 2:
        return None
    x, y, w, h = rect
    # 纵向自适应
    vmin = min(data)
    vmax = max(data)
    if vmin == vmax:
        vmin -= 1.0
        vmax += 1.0
    scale = h / (vmax - vmin)
    # 将 data 映射到折线
    n = len(data)
    # 在可视宽度内均匀分布最近 PLOT_HISTORY 个点
    pts = []
    for i, v in enumerate(list(data)[-PLOT_HISTORY:]):
        px = x + int(w * i / max(1, PLOT_HISTORY - 1))
        py = y + h - int((v - vmin) * scale)
        pts.append((px, py))
    # 折线
    pygame.draw.lines(screen, color, False, pts, 2)
    # 标注最新值
    latest = data[-1]
    draw_text(x + 6, y + 6, f"{name}: {latest:.3f}  [min={vmin:.3f}, max={vmax:.3f}]", font=font_small)


def get_channels_to_draw():
    if selected_channels:
        return [c for c in selected_channels if c in channel_buffers]
    # 自动选择：按出现顺序取最多 MAX_CHANNELS_TO_DRAW 个
    keys = list(channel_buffers.keys())[:MAX_CHANNELS_TO_DRAW]
    return keys


def main():
    global aaaa, bbbb, cccc, dddd, target_depth, isHovering, selected_channels
    connect_to_pi()

    pygame.joystick.init()
    if pygame.joystick.get_count() == 0:
        print("No joystick found.")
        return
    js = pygame.joystick.Joystick(0)
    js.init()
    print(f"Using joystick: {js.get_name()}")

    last_cmd_hex = None
    last_depth_tick = 0.0
    last_servo_tick = 0.0
    hat_state = (0, 0)

    # 最近一次已发送的平面向量，避免频繁冗余发送
    last_planar_xy = (None, None)
    # 最近一次已发送的 yaw 角速度（度/秒），避免抖动
    last_yaw_rate = None
    last_depth_force = None

    running = True
    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            if event.type == pygame.JOYHATMOTION:
                hat_state = js.get_hat(0)
            if event.type == pygame.JOYBUTTONDOWN:
                if event.button == 7:  # 悬浮开关
                    send_pkt(CMD_TOGGLE_FLOAT)
                    isHovering = not isHovering
                if event.button == 0:
                    send_pkt(CMD_TOGGLE_YAW_LOOP)
            if event.type == pygame.KEYDOWN:
                # 数字键 1-9 选择要显示的通道（按照发现的顺序编号）
                if pygame.K_1 <= event.key <= pygame.K_9:
                    idx = event.key - pygame.K_1
                    keys = list(channel_buffers.keys())
                    if idx < len(keys):
                        ch = keys[idx]
                        if ch in selected_channels:
                            selected_channels.remove(ch)
                        else:
                            selected_channels.append(ch)
                            if len(selected_channels) > MAX_CHANNELS_TO_DRAW:
                                selected_channels = selected_channels[-MAX_CHANNELS_TO_DRAW:]
                # 按 0 清空选择，回到自动模式
                if event.key == pygame.K_0:
                    selected_channels = []

        try:
            leftH = js.get_axis(0)
            leftV = js.get_axis(1)
            rightH = js.get_axis(2)
            rightV = js.get_axis(3)
        except Exception:
            leftH = leftV = rightH = rightV = 0.0

        # —— 平面推力百分比：发送 0x30 [int8 x, int8 y] ——
        x_pct = calculate_speed(leftH, DIRECTION_THRESHOLD)
        y_pct = -calculate_speed(leftV, DIRECTION_THRESHOLD)
        if isHovering:
            payload = i8(x_pct) + i8(y_pct)
            send_pkt(CMD_SET_PLANAR_VEL, payload)
            last_planar_xy = (x_pct, y_pct)

        # —— 右摇杆水平控制“偏航角速度” → 0x23 [int16 deg/s] ——
        yaw_rate_cmd = 0
        if not (-ROTATE_THRESHOLD < rightH < ROTATE_THRESHOLD):
            yaw_rate_cmd = calculate_speed(rightH, ROTATE_THRESHOLD)
        if isHovering:
            send_pkt(CMD_SET_YAW_RATE, i16le(yaw_rate_cmd))
            last_yaw_rate = yaw_rate_cmd

        now = time.time()

        depth_force_cmd = 0
        if not (-DEPTH_THRESHOLD < rightV < DEPTH_THRESHOLD):
            depth_force_cmd = calculate_speed(rightV, DEPTH_THRESHOLD)
        if isHovering:
            send_pkt(CMD_SET_DEPTH_FORCE, i16le(depth_force_cmd))
            last_depth_force = depth_force_cmd

        # 舵机控制 (bbbb, cccc)
        if hat_state != (0, 0) and (now - last_servo_tick >= SERVO_REPEAT_INTERVAL):
            hx, hy = hat_state
            changed = False
            if hy == 1:
                bbbb = min(bbbb + SERVO_STEP, PWM_MAX)
                changed = True
            elif hy == -1:
                bbbb = max(bbbb - SERVO_STEP, PWM_MIN)
                changed = True
            if hx == 1:
                cccc = min(cccc + SERVO_STEP, PWM_MAX)
                changed = True
            elif hx == -1:
                cccc = max(cccc - SERVO_STEP, PWM_MIN)
                changed = True
            last_servo_tick = now

        # UI 绘制
        screen.fill((255, 255, 255))

        # 左侧：控制与发送日志
        y = 8
        y = draw_text(10, y, f"Left(H,V)=({leftH:+.2f},{leftV:+.2f})  Right(H,V)=({rightH:+.2f},{rightV:+.2f})")
        y = draw_text(10, y, f"Planar: x={x_pct}, y={y_pct}   Yaw rate cmd: {yaw_rate_cmd:+d} deg/s")
        y = draw_text(10, y, f"Depth target: {target_depth} (0.1 m units)")
        deg_b = pwm_to_deg(bbbb)
        deg_c = pwm_to_deg(cccc)
        y = draw_text(10, y, f"Servos: x={deg_b:.1f}°  y={deg_c:.1f}°")

        y = draw_text(10, y + 8, "Sent commands (latest first):")
        for line in list(send_log):
            y = draw_text(14, y, line)

        # 右侧：实时曲线区域
        plot_rect = (320, 20, 560, 460)
        draw_axes(plot_rect)

        # 要绘制的通道
        chs = get_channels_to_draw()
        # 列出所有已发现通道与索引（用于快捷键提示）
        key_list_y = draw_text(320, 486, "Channels:")
        keys_all = list(channel_buffers.keys())
        for i, ch in enumerate(keys_all[:9]):
            key_list_y = draw_text(400 + (i % 3) * 150, 486 + (i // 3) * 16, f"[{i+1}] {ch}")

        # 绘制最多 MAX_CHANNELS_TO_DRAW 条折线
        for idx, ch in enumerate(chs):
            color = COLOR_TABLE[idx % len(COLOR_TABLE)]
            data = channel_buffers.get(ch, None)
            if data:
                draw_series(plot_rect, ch, data, color)

        # 提示
        draw_text(320, 4, "Press 1..9 to (un)select channels, 0 to auto", font=font_small, color=(80, 80, 80))

        pygame.display.flip()
        clock.tick(FPS)

    try:
        sock.close()
    except Exception:
        pass
    pygame.quit()


if __name__ == "__main__":
    main()
