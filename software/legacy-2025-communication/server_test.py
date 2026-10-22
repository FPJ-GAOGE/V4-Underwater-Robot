import socket

# 创建 TCP 套接字
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# 获取树莓派的 IP 地址（例如：192.168.1.100）和端口号
host = '192.168.106.76'  # 监听所有可用的网络接口
port = 12345  # 服务器端口号

# 绑定服务器地址和端口
server_socket.bind((host, port))

# 开始监听客户端连接
server_socket.listen(1)
print(f"等待客户端连接...")

# 接受客户端的连接
client_socket, client_address = server_socket.accept()
print(f"已连接到客户端: {client_address}")

# 进行双向通信
try:
    with open("image.jpg", "rb") as f:
        while chunk := f.read(4096):
            client_socket.send(chunk)

    print("图片发送完毕。")
    # with open("image.jpg","wb") as f:
    #     count = 0
    #     while True:
    #         data = client_socket.recv(4096)
    #         if not data:
    #             break
    #         f.write(data)
    #         count += 1
    #         print(f"接收图片数据包 {count}")
    #     f.close()
    #
    # print("接收图片成功！")
    # while True:
    #     # 从客户端接收数据
    #     client_message = client_socket.recv(1024).decode('utf-8')
    #     if not client_message:
    #         break
    #     print(f"收到客户端消息: {client_message}")
    #
    #     # 发送响应给客户端
    #     server_response = "服务器收到你的消息！"
    #     client_socket.sendall(server_response.encode('utf-8'))

except KeyboardInterrupt:
    print("服务器退出。")

# 关闭连接
client_socket.close()
server_socket.close()
