#include <SPI.h>
#include <WiFiNINA.h>
#include "HID-Project.h"

int status = WL_IDLE_STATUS;
const char* ssid = "MotionControl";
const char* password = "frenchpilote"; 
WiFiServer server(80);

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("Access Point Enabled");

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  Serial.print("Creating access point named: ");
  Serial.println(ssid);

  status = WiFi.beginAP(ssid, password);
  if (status != WL_AP_LISTENING) {
    Serial.println("Creating access point failed");
    while (true);
  }

  delay(10000); // wait for connection
  server.begin();
  Gamepad.begin();
}

void loop() {
  if (status != WiFi.status()) {
    status = WiFi.status();
    if (status == WL_AP_CONNECTED) {
      Serial.println("Device connected to AP");
    } else {
      Serial.println("Device disconnected from AP");
    }
  }

  WiFiClient client = server.available();
  if (client) {
    Serial.println("New Client Connected");
    String data = "";

    while (client.connected()) {
      if (client.available()) {
        data = client.readStringUntil('\n');

        int commaIndex = data.indexOf(',');
        if (commaIndex == -1) {
          // Data from Left Hand Controller
          float thrust = data.toFloat();
          // Update gamepad with thrust value
          // Example: Gamepad.setThrust(thrust);
        } else {
          // Data from Right Hand Controller
          float x = getValue(data, ',', 0).toFloat();
          float y = getValue(data, ',', 1).toFloat();
          float z = getValue(data, ',', 2).toFloat();
          // Update gamepad with x, y, z values
          // Example: Gamepad.setX(x); Gamepad.setY(y); Gamepad.setZ(z);
        }
        Gamepad.write();
        Serial.println(data);
      }
    }

    Serial.println("Client Disconnected.");
    client.stop();
  }
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
