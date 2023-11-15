#include <SPI.h>
#include <WiFiNINA.h>
#include "HID-Project.h"

const char* ssid = "MotionControl";
const char* password = "frenchpilote"; 
WiFiServer server(80);

WiFiClient clients[2];
String leftHandData = "";   // Stores the latest data from Left Hand Controller
String rightHandData = "";  // Stores the latest data from Right Hand Controller

void setup() {
  Serial.begin(9600);
  WiFi.beginAP(ssid, password);
  while (WiFi.status() == WL_NO_MODULE) {
    delay(500);
  }
  server.begin();
  Gamepad.begin();
}

void loop() {
  // Check for new client connections
  WiFiClient newClient = server.available();
  if (newClient) {
    // Check if new connection is from a known client or a new one
    bool knownClient = false;
    for (int i = 0; i < 2; i++) {
      if (clients[i] && clients[i].remoteIP() == newClient.remoteIP()) {
        knownClient = true;
        break;
      }
    }
    
    // If it's a new client, add it to the list
    if (!knownClient) {
      for (int i = 0; i < 2; i++) {
        if (!clients[i]) {
          clients[i] = newClient;
          Serial.println("New client connected");
          break;
        }
      }
    }
  }

  // Read data from each client
  for (int i = 0; i < 2; i++) {
    if (clients[i] && clients[i].available()) {
      String data = clients[i].readStringUntil('\n');
      Serial.println(data);

      if (data.startsWith("L:")) {
        leftHandData = data.substring(2); // Extract thrust
      } else if (data.startsWith("R:")) {
        rightHandData = data.substring(2); // Extract roll, pitch, yaw
      }

      // If data from both controllers is available, process it
      if (leftHandData.length() > 0 && rightHandData.length() > 0) {
        processAndSendData();
        leftHandData = "";
        rightHandData = "";
      }
    }
  }
}

void processAndSendData() {
  // Extract thrust, roll, pitch, yaw values
  float thrust = leftHandData.toFloat();
  float roll = getValue(rightHandData, ',', 0).toFloat();
  float pitch = getValue(rightHandData, ',', 1).toFloat();
  float yaw = getValue(rightHandData, ',', 2).toFloat();

  // Send data to cfclient via HID
  // Example: Gamepad.setX(roll); Gamepad.setY(pitch); Gamepad.setZ(yaw); Gamepad.setThrottle(thrust);
  Gamepad.write();

  Serial.print("Sent Combined Data - Thrust: "); Serial.print(thrust);
  Serial.print(", Roll: "); Serial.print(roll);
  Serial.print(", Pitch: "); Serial.print(pitch);
  Serial.print(", Yaw: "); Serial.println(yaw);
}

String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
        found++;
        strIndex[0] = strIndex[1] + 1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
