#include <SPI.h>
#include <WiFiNINA.h>
#include <Arduino_LSM6DS3.h>

// Replace with your network credentials
const char* ssid = "MotionControl";
const char* password = "frenchpilote";

// Replace with the IP address of the PC Controller
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
  float x, y, z;
  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(x, y, z);
    String data = String(x);
    Serial.println(data);
    client.println(data);
  }
  delay(100); // Adjust delay for data transmission rate
}

