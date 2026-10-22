from client import *
import struct
client = Client()
print("starting client")
# 初始化一个空的字节流用来接受数据
payload_size = struct.calcsize("!I")  # 4字节
data = b""

try:
    while True:
        frame = client.receive_image()
        cv2.imshow('Received Stream', frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

finally:
    client.close()
    cv2.destroyAllWindows()