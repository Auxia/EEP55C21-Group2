#include <SPI.h>
#include <WiFiNINA.h>
#include "HID-Project.h"

int status = WL_IDLE_STATUS;

const char* ssid = "MotionControl";
const char* password = "frenchpilote"; 

WiFiServer server(80);

void setup() {
  Serial.begin(9600);

  while(!Serial) {
    ;
  }

  Serial.println("Access Point Enabled");

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
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
    // don't continue
    while (true);
  }

  // wait 10 seconds for connection:
  delay(10000);

  server.begin();
  Gamepad.begin();
}

void loop() {
  // compare the previous status to the current status
  if (status != WiFi.status()) {
    // it has changed update the variable
    status = WiFi.status();

    if (status == WL_AP_CONNECTED) {
      // a device has connected to the AP
      Serial.println("Device connected to AP");
    } else {
      // a device has disconnected from the AP, and we are back in listening mode
      Serial.println("Device disconnected from AP");
    }
  }

  WiFiClient client = server.available();
  if (client) {
    Serial.println("New Client Connected");
    String data = ""; // String to store incoming data from client

    // Loop while the client is connected
    while (client.connected()) {
      if (client.available()) {
        String data = client.readStringUntil('\n');
        Serial.println(data);
        float x = data.toFloat();
        Gamepad.xAxis(x);
        Gamepad.write();
        }
      }

    // Client has disconnected
    Serial.println("Client Disconnected.");
    client.stop();
  }
}

