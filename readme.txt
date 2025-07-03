# Hand Gesture Controlled Robotic Arm with ESP32

## 📦 Project Structure
- `esp32_code/`: Arduino code for ESP32 (controls motors and servos via HTTP)
- `python_controller/`: Python code to track hand using webcam and control ESP32

## 🛠 Requirements

### ESP32
- Board: ESP32
- Libraries:
  - ESPAsyncWebServer
  - ESP32Servo

### Python
```bash
pip install opencv-python mediapipe numpy requests
