import socket
import threading

# ===== 配置部分 =====
RASPBERRY_IP = '192.168.138.2'  # ← 修改为树莓派的IP
RASPBERRY_PORT = 5000

def recv_from_pi(sock):
    """接收从树莓派（即单片机返回的数据）"""
    while True:
        try:
            data = sock.recv(1024)
            if not data:
                print("连接已关闭。")
                break
            print(f"[来自单片机] {data.decode(errors='ignore')}")
        except Exception as e:
            print("接收出错:", e)
            break

def main():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        sock.connect((RASPBERRY_IP, RASPBERRY_PORT))
        print(f"已连接到树莓派 {RASPBERRY_IP}:{RASPBERRY_PORT}")
    except Exception as e:
        print("无法连接:", e)
        return

    # 启动接收线程
    threading.Thread(target=recv_from_pi, args=(sock,), daemon=True).start()

    print("请输入要发送给单片机的数据（Ctrl+C 退出）：")
    while True:
        try:
            msg = input()
            if not msg:
                continue
            sock.sendall(msg.encode())
        except KeyboardInterrupt:
            print("\n用户退出。")
            break
        except Exception as e:
            print("发送出错:", e)
            break

    sock.close()

if __name__ == '__main__':
    main()
