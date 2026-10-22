# -*- coding: utf-8 -*-
import os
import socket
import time
import pygame
import struct
import threading
from collections import deque

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
PROPELLER_FRAME_HEAD = 0x02
SERVO_FRAME_HEAD = 0x03
CMD_SET_DEPTH_VALUE = 0x00  # [int16 depth_x10]
CMD_SET_DEPTH_INCRE = 0x01  # [int16 depth_x10]
CMD_SET_DEPTH_FORCE = 0x02  # [int16 force]
CMD_TOGGLE_FLOAT = 0x10  #
CMD_TOGGLE_YAW_LOOP = 0x20  #
CMD_SERVO_START = 0x00  #
CMD_SERVO_STOP = 0x01  #
CMD_TOGGLE_YAW_DOWN = 0x01   #
CMD_SET_YAW_VALUE = 0x21  # [int16 deg]
CMD_INC_YAW_INCRE = 0x22  # [int16 deg]
CMD_SET_YAW_RATE = 0x23  # [int16 deg_per_s]，0=停转并锁角
CMD_SET_PLANAR_VEL = 0x30  # [int8 x, int8 y]，-99..99
isHovering = False

pygame.init()
screen = pygame.display.set_mode((600, 340))  # 适当增加高度显示速度信息
pygame.display.set_caption("Joystick TCP Controller")
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


recv_log = deque(maxlen=20)
rx_thread = None
rx_stop = threading.Event()
log_file_path = "recv_log.txt"


def log_recv(msg):
    ts = time.strftime("%H:%M:%S")
    line = f"{ts}  {msg}"
    recv_log.appendleft(line)

    # 同时写入文件
    try:
        with open(log_file_path, "a", encoding="utf-8") as f:
            f.write(line + "\n")
    except Exception as e:
        print(f"[FILE] 写入日志失败: {e}")


def start_rx_thread():
    global rx_thread, rx_stop
    rx_stop.clear()
    rx_thread = threading.Thread(target=recv_worker, daemon=True)
    rx_thread.start()


def recv_worker():
    """后台线程：接收下位机发来的字符并写入文件"""
    buf = b""
    while not rx_stop.is_set():
        try:
            data = sock.recv(1024)
            if not data:
                log_recv("# 连接丢失，正在重连...")
                connect_to_pi()
                continue
            buf += data
            while b"\n" in buf:
                line, buf = buf.split(b"\n", 1)
                text = line.decode(errors="ignore").strip("\r")
                if text:
                    log_recv(text)  # 同时显示与写文件
        except Exception as e:
            log_recv(f"# 接收错误: {e}")
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


def send_pkt(head,code, payload=b""):
    """发送一帧二进制：head(1) + code(1) + payload(n)"""
    global sock
    frame = bytes([head, code]) + payload
    try:
        sock.sendall(frame)
        # 日志以十六进制显示
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


def main():
    global aaaa, bbbb, cccc, dddd, target_depth, isHovering
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
                    send_pkt(PROPELLER_FRAME_HEAD,CMD_TOGGLE_FLOAT)
                    isHovering = not isHovering
                if event.button == 0:
                    send_pkt(PROPELLER_FRAME_HEAD,CMD_TOGGLE_YAW_LOOP)
                if event.button == 1:
                    send_pkt(SERVO_FRAME_HEAD,CMD_SERVO_START)
                if event.button == 2:
                    send_pkt(SERVO_FRAME_HEAD,CMD_SERVO_STOP)

        try:
            leftH = js.get_axis(0)
            leftV = js.get_axis(1)
            rightH = js.get_axis(2)
            rightV = js.get_axis(3)
        except Exception:
            leftH = leftV = rightH = rightV = 0.0

        # —— 平面推力百分比：发送 0x30 [int8 x, int8 y] ——
        # 你的 MCU 会把 -99..99 的百分比映射到 PWM 增量
        x_pct = calculate_speed(leftH, DIRECTION_THRESHOLD)
        y_pct = -calculate_speed(leftV, DIRECTION_THRESHOLD)
        # 为避免占用带宽，只有变化才发
        if isHovering and (x_pct, y_pct) != last_planar_xy:
            payload = i8(x_pct) + i8(y_pct)
            send_pkt(PROPELLER_FRAME_HEAD,CMD_SET_PLANAR_VEL, payload)
            last_planar_xy = (x_pct, y_pct)

        # —— 右摇杆水平控制“偏航角速度” → 0x23 [int16 deg/s] ——
        # 右杆>0：顺时针为正；右杆<0：逆时针为负（与之前 E/Q 的方向保持一致）
        yaw_rate_cmd = 0
        if not (-ROTATE_THRESHOLD < rightH < ROTATE_THRESHOLD):
            yaw_rate_cmd = calculate_speed(rightH, ROTATE_THRESHOLD)

        if isHovering and yaw_rate_cmd != last_yaw_rate:
            send_pkt(PROPELLER_FRAME_HEAD,CMD_SET_YAW_RATE, i16le(yaw_rate_cmd))
            last_yaw_rate = yaw_rate_cmd

        now = time.time()

        depth_force_cmd = 0
        if not (-DEPTH_THRESHOLD < rightV < DEPTH_THRESHOLD):
            depth_force_cmd = calculate_speed(rightV, DEPTH_THRESHOLD)

        if isHovering and depth_force_cmd != last_depth_force:
            send_pkt(PROPELLER_FRAME_HEAD,CMD_SET_DEPTH_FORCE, i16le(depth_force_cmd))
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
            # if changed:
            # send_tcp(f"MOT:{aaaa},{bbbb},{cccc},{dddd}")
            last_servo_tick = now

        # 心跳包保持原逻辑
        # if now - last_hb >= HEARTBEAT_INTERVAL:
        #     send_tcp(HEARTBEAT_MSG)
        #     last_hb = now

        # UI显示新增速度信息
        screen.fill((255, 255, 255))
        y = 8
        y = draw_text(10, y, f"Left(H,V)=({leftH:+.2f},{leftV:+.2f})  Right(H,V)=({rightH:+.2f},{rightV:+.2f})")
        y = draw_text(10, y,
                      f"Planar: x={x_pct}, y={y_pct}   Yaw rate cmd: {yaw_rate_cmd:+d} deg/s")
        y = draw_text(10, y, f"Depth target: {target_depth} (0.1 m units)")
        deg_b = pwm_to_deg(bbbb)
        deg_c = pwm_to_deg(cccc)
        y = draw_text(10, y, f"Servos: x={deg_b:.1f}°  y={deg_c:.1f}°")

        # 绘制发送日志
        y = draw_text(10, y + 8, "Sent commands (latest first):")
        for line in list(send_log):
            y = draw_text(14, y, line)

        # 绘制接收日志
        y = draw_text(320, 8, "Received messages:")
        for line in list(recv_log):
            y = draw_text(324, y, line)
        pygame.display.flip()
        clock.tick(FPS)

    try:
        sock.close()
    except Exception:
        pass
    pygame.quit()


if __name__ == "__main__":
    main()
