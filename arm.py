import cv2
import mediapipe as mp
import numpy as np
import requests
import time

ESP32_IP = "http://192.168.137.158"  # Replace with your ESP32 IP

mp_hands = mp.solutions.hands
hands = mp_hands.Hands(min_detection_confidence=0.8)
mp_drawing = mp.solutions.drawing_utils

cap = cv2.VideoCapture(0)
ws, hs = 1280, 720
cap.set(3, ws)
cap.set(4, hs)

def control_servo(endpoint, value):
    url = f"{ESP32_IP}/{endpoint}?value={value}"
    try:
        requests.get(url, timeout=0.1)
    except:
        pass

def fingers_up(lm_list, hand_type):
    fingers = []
    if hand_type == "Right":
        fingers.append(1 if lm_list[4][0] > lm_list[3][0] else 0)
    else:
        fingers.append(1 if lm_list[4][0] < lm_list[3][0] else 0)
    for tip in [8, 12, 16, 20]:
        fingers.append(1 if lm_list[tip][1] < lm_list[tip - 2][1] else 0)
    return fingers

def move_servo_smooth(endpoint, start_angle, end_angle, delay=0.005):
    step = 1 if end_angle > start_angle else -1
    for angle in range(start_angle, end_angle + step, step):
        control_servo(endpoint, angle)
        time.sleep(delay)

def move_servo_smooth_slow(endpoint, start_angle, end_angle, delay=0.05):
    move_servo_smooth(endpoint, start_angle, end_angle, delay)

# Default positions
default_servo_x = 85
default_servo_y = 94
default_servo_z = 90
default_servo_d = 107

move_servo_smooth_slow('servoX', default_servo_x, default_servo_x)
move_servo_smooth_slow('servoY', default_servo_y, default_servo_y)
move_servo_smooth_slow('servoZ', default_servo_z, default_servo_z)
move_servo_smooth_slow('servoD', default_servo_d, default_servo_d)

print("Default angles set.")

while cap.isOpened():
    success, img = cap.read()
    img = cv2.flip(img, 1)
    img_rgb = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
    results = hands.process(img_rgb)
    img = cv2.cvtColor(img_rgb, cv2.COLOR_RGB2BGR)

    lm_list = []
    if results.multi_hand_landmarks:
        for hand_landmarks in results.multi_hand_landmarks:
            mp_drawing.draw_landmarks(img, hand_landmarks, mp_hands.HAND_CONNECTIONS)
        hand_landmarks = results.multi_hand_landmarks[0]
        h, w, c = img.shape
        for lm in hand_landmarks.landmark:
            lm_list.append([int(lm.x * w), int(lm.y * h), lm.z])

        hand_type = results.multi_handedness[0].classification[0].label
        px, py, pz = lm_list[8]

        servo_x = int(np.interp(px, [0, ws], [180, 0]))
        servo_y = int(np.interp(py, [0, hs], [0, 180]))
        move_servo_smooth('servoX', default_servo_x, servo_x)
        move_servo_smooth('servoY', default_servo_y, servo_y)

        fingers = fingers_up(lm_list, hand_type)
        servo_z = 0 if fingers[0] == 0 else 90
        move_servo_smooth('servoZ', default_servo_z, servo_z)

        servo_d = int(np.interp(pz, [-0.2, 0.2], [180, 0]))
        move_servo_smooth('servoD', default_servo_d, servo_d)

        default_servo_x = servo_x
        default_servo_y = servo_y
        default_servo_z = servo_z
        default_servo_d = servo_d

        cv2.putText(img, f'X: {servo_x} Y: {servo_y} Z: {servo_z} D: {servo_d}', (20, 50),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.8, (255, 0, 0), 2)

    cv2.imshow("Hand Tracking Control", img)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
