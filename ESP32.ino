#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESP32Servo.h>

// WiFi Credentials
const char* ssid = "Mahesh";
const char* password = "Mahesh123";
AsyncWebServer server(80);

// Car Motor A pins
const int motorA_IN1 = 13;
const int motorA_IN2 = 27;
const int motorA_PWM = 14;

// Car Motor B pins
const int motorB_IN1 = 26;
const int motorB_IN2 = 25;
const int motorB_PWM = 12;

// Servo motors
Servo servoX, servoY, servoZ, servoD;

void setup() {
  Serial.begin(115200);

  // Initialize car motor pins
  pinMode(motorA_IN1, OUTPUT);
  pinMode(motorA_IN2, OUTPUT);
  pinMode(motorB_IN1, OUTPUT);
  pinMode(motorB_IN2, OUTPUT);
  pinMode(motorA_PWM, OUTPUT);
  pinMode(motorB_PWM, OUTPUT);

  // PWM setup
  ledcSetup(0, 5000, 8); ledcAttachPin(motorA_PWM, 0);
  ledcSetup(1, 5000, 8); ledcAttachPin(motorB_PWM, 1);

  // Attach servos
  servoX.attach(15);
  servoY.attach(18);
  servoZ.attach(23);
  servoD.attach(19);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000); Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  Serial.println(WiFi.localIP());

  // Car motor control
  server.on("/car", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("direction")) {
      String direction = request->getParam("direction")->value();
      int speed = request->hasParam("speed") ? request->getParam("speed")->value().toInt() : 255;

      if (direction == "left") {
        digitalWrite(motorA_IN1, LOW); digitalWrite(motorA_IN2, HIGH);
        digitalWrite(motorB_IN1, HIGH); digitalWrite(motorB_IN2, LOW);
      } else if (direction == "right") {
        digitalWrite(motorA_IN1, HIGH); digitalWrite(motorA_IN2, LOW);
        digitalWrite(motorB_IN1, LOW); digitalWrite(motorB_IN2, HIGH);
      } else if (direction == "up") {
        digitalWrite(motorA_IN1, HIGH); digitalWrite(motorA_IN2, LOW);
        digitalWrite(motorB_IN1, HIGH); digitalWrite(motorB_IN2, LOW);
      } else if (direction == "down") {
        digitalWrite(motorA_IN1, LOW); digitalWrite(motorA_IN2, HIGH);
        digitalWrite(motorB_IN1, LOW); digitalWrite(motorB_IN2, HIGH);
      } else {
        digitalWrite(motorA_IN1, LOW); digitalWrite(motorA_IN2, LOW);
        digitalWrite(motorB_IN1, LOW); digitalWrite(motorB_IN2, LOW);
        speed = 0;
      }

      ledcWrite(0, speed);
      ledcWrite(1, speed);
      request->send(200, "text/plain", "Car moved " + direction);
    } else {
      request->send(400, "text/plain", "Missing direction parameter");
    }
  });

  // Servo routes
  auto handleServo = [](AsyncWebServerRequest *request, Servo &servo, const String &name) {
    if (request->hasParam("value")) {
      int angle = request->getParam("value")->value().toInt();
      servo.write(angle);
      request->send(200, "text/plain", name + " moved to " + String(angle));
    } else {
      request->send(400, "text/plain", "Missing value parameter");
    }
  };

  server.on("/servoX", HTTP_GET, [](AsyncWebServerRequest *req){ handleServo(req, servoX, "Servo X"); });
  server.on("/servoY", HTTP_GET, [](AsyncWebServerRequest *req){ handleServo(req, servoY, "Servo Y"); });
  server.on("/servoZ", HTTP_GET, [](AsyncWebServerRequest *req){ handleServo(req, servoZ, "Servo Z"); });
  server.on("/servoD", HTTP_GET, [](AsyncWebServerRequest *req){ handleServo(req, servoD, "Servo D"); });

  server.begin();
  Serial.println("Server started");
}

void loop() {}
