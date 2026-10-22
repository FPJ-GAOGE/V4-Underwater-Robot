import socket
import struct
import pickle
import cv2
class Client:

    def __init__(self,host='192.168.177.76',port=12345):
        self.host = host
        self.port = port
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.payload_size = struct.calcsize("!I")

        try:
            self.sock.connect((self.host, self.port))
            print("成功连接到服务器！")
        except socket.timeout:
            print("错误：连接超时，请检查服务器地址和端口是否正确，或确认网络状态。")
        except ConnectionRefusedError:
            print("错误：连接被拒绝，请确保服务器已启动并在监听指定端口。")
        except socket.gaierror:
            print("错误：主机名解析失败，请检查服务器地址是否正确。")
        except Exception as e:
            print(f"未知连接错误：{str(e)}")


    def send_string(self,string):
        self.sock.sendall(string.encode('utf-8'))

    def receive_image(self):
        data = b''
        while len(data) < self.payload_size:
            data += self.sock.recv(4096)
        packed_size = data[:self.payload_size]
        data = data[self.payload_size:]
        size = struct.unpack("!I", packed_size)[0]

        # 获取实际数据
        while len(data) < size:
            data += self.sock.recv(4096)
        frame_data = data[:size]
        data = data[size:]
        data = self.sock.recv(1024)

        buffer = pickle.loads(frame_data)
        frame = cv2.imdecode(buffer, cv2.IMREAD_COLOR)
        return frame

    def close(self):
        self.sock.close()
