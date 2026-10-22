from server import *
import cv2
import pickle

server = Server()

print("Server is running...")

cap = cv2.VideoCapture(0)
try:
    while cap.isOpened():
        ret, frame = cap.read()
        if not ret:
            print("Failed to capture frame")
            break

        server.send_image(frame)

finally:
    cap.release()
    server.close()



