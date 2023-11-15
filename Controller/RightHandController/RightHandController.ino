#include <SPI.h>
#include <WiFiNINA.h>
#include <Arduino_LSM6DS3.h>

const char* ssid = "MotionControl";
const char* password = "frenchpilote";
IPAddress server(192, 168, 4, 1);
WiFiClient client;

void setup() {
  Serial.begin(9600);
  while (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    delay(1000);
  }
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  if (client.connect(server, 80)) {
    Serial.println("Connected to server");
  } else {
    Serial.println("Failed to connect to server");
  }
}

void loop() {
  float roll, pitch, yaw;
  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(roll, pitch, yaw);
    client.println("R:" + String(roll) + "," + String(pitch) + "," + String(yaw));
  }
  delay(100);
}
