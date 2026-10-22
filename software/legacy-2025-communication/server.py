import socket
import struct
import cv2
import pickle
class Server:

    def __init__(self,host='192.168.177.76',port=12345):
        self.host = host
        self.port = port
        self.sock = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
        self.sock.bind((self.host,self.port))
        self.sock.listen(1)
        print(f"Server is listening on {self.host}:{self.port}")
        self.client_socket,self.client_address = self.sock.accept()
        print(f"Connection established with {self.client_address}")

    def send_image(self,image):
        _, buffer = cv2.imencode('.jpg', image, [cv2.IMWRITE_JPEG_QUALITY, 80])
        data = pickle.dumps(buffer)
        size = len(image)
        self.client_socket.sendall(struct.pack('!I',size))
        self.client_socket.sendall(image)
        print("Image sent successfully")

    def receive_string(self):
        data = self.client_socket.recv(1024).decode()
        return data

    def close(self):
        self.client_socket.close()
        self.sock.close()