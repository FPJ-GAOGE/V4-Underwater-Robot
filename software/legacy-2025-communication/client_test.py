import socket

# 设置树莓派的 IP 地址和端口号
host = '192.168.177.76'  # 替换为树莓派的实际 IP 地址
port = 12345  # 服务器端口号

# 创建 TCP 套接字
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# 连接到服务器（树莓派）
client_socket.connect((host, port))

# 进行双向通信
try:
    with open("return.jpg", "wb") as f:
        while True:
            data = client_socket.recv(4096)
            if not data:
                break
            f.write(data)
        f.close()
        print("图片接收完毕。")
    # with open("test.jpg", "rb") as f:
    #     while chunk := f.read(4096):
    #         client_socket.send(chunk)
    #
    # print("图片发送完毕。")
    # while True:
    #     # 发送消息给服务器
    #     client_message = "Hello, server!"
    #     client_socket.sendall(client_message.encode('utf-8'))
    #
    #     # 接收来自服务器的响应
    #     server_response = client_socket.recv(1024).decode('utf-8')
    #     print(f"收到服务器响应: {server_response}")
    #
    #     if client_message.lower() == 'exit':
    #         break

except KeyboardInterrupt:
    print("客户端退出。")

# 关闭连接
client_socket.close()
