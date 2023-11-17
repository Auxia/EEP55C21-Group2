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
      // Serial.println(data);

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
  // float thrust = leftHandData.toFloat();
  float thrust = getValue(leftHandData, ',', 0).toFloat();
  float yaw = getValue(leftHandData, ',', 1).toFloat();
  float roll = getValue(rightHandData, ',', 0).toFloat();
  float pitch = getValue(rightHandData, ',', 1).toFloat();
  // float yaw = getValue(rightHandData, ',', 2).toFloat();

  // int gamepadThrust = mapIMUToGamepadRange(thrust, 5, -127, 127);
  int gamepadThrust = mapIMUToGamepadRangeNonLinear(thrust, -127, -90, 110, 127);
  int gamepadRoll = mapIMUToGamepadRangeNonLinear(roll, -32768, -25000, 25000, 32767);
  int gamepadPitch = mapIMUToGamepadRangeNonLinear(pitch, -32768, -25000, 25000, 32767);

  // Going to have to implement Yaw using the left controller itself
  int gamepadYaw = mapIMUToGamepadRangeNonLinear(yaw, -32768, -25000, 25000, 32767);
  
  // The values are inverted here as compared to how it should be for a controller naturally
  gamepadPitch = -gamepadPitch;

  // Send data to cfclient via HID
  // Example: Gamepad.setX(roll); Gamepad.setY(pitch); Gamepad.setZ(yaw); Gamepad.setThrottle(thrust);
  Gamepad.zAxis(gamepadThrust);
  Gamepad.rxAxis(gamepadRoll);
  Gamepad.ryAxis(gamepadPitch);
  Gamepad.xAxis(gamepadYaw);
  Serial.println("Thrust");
  Serial.println(gamepadThrust);
  Serial.println("Roll");
  Serial.println(gamepadRoll);
  Serial.println("Pitch");
  Serial.println(gamepadPitch);
  Serial.println("Yaw");
  Serial.println(gamepadYaw);
  Gamepad.write();

  // Serial.print("Sent Combined Data - Thrust: "); Serial.print(thrust);
  // Serial.print(", Roll: "); Serial.print(roll);
  // Serial.print(", Pitch: "); Serial.print(pitch);
  // Serial.print(", Yaw: "); Serial.println(yaw);
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

// Function to map IMU thrust data to Gamepad range
int mapIMUToGamepadRange(float IMUData, float sensitivity, int minGamepad, int maxGamepad) {
    // Define the IMU data range
    float minIMUData = -1.0; // Minimum thrust value from IMU
    float maxIMUData = 1.0;  // Maximum thrust value from IMU

    float amplifiedData = IMUData * sensitivity;

    // Constrain the amplified data to the -1 to 1 range
    amplifiedData = constrain(amplifiedData, minIMUData, maxIMUData);

    // Map the IMU thrust value to the Gamepad range
    // return (int)((thrust - minThrust) / (maxThrust - minThrust) * (maxGamepad - minGamepad) + minGamepad);
    return (int)((amplifiedData + 1) / 2 * (maxGamepad - minGamepad) + minGamepad);
}

// Function to map IMU data to Gamepad range with non-linear scaling
int mapIMUToGamepadRangeNonLinear(float IMUData, int minGamepad, int midMinGamepad, int midMaxGamepad, int maxGamepad) {
  float correctedData = IMUData - 0;
  float minIMUData = -1.0;
  float maxIMUData = 1.0;

  float scaledData = constrain(correctedData, minIMUData, maxIMUData);

  // Determine the percentage of the way through the range
  float percent = (scaledData - minIMUData) / (maxIMUData - minIMUData);

  // Check if the scaled data is in the inner slow range or the outer fast range
  if (scaledData > 0) {
      if (scaledData <= (midMaxGamepad - midMinGamepad) / (float)(maxGamepad - minGamepad)) {
          // Scale slowly in the positive inner range
          return (int)(percent * (midMaxGamepad - midMinGamepad) + midMinGamepad);
      } else {
          // Scale quickly in the positive outer range
          return (int)(percent * (maxGamepad - midMaxGamepad) + midMaxGamepad);
      }
  } else {
      if (scaledData <= (midMinGamepad - minGamepad) / (float)(maxGamepad - minGamepad)) {
          // Scale slowly in the negative inner range
          return (int)(percent * (midMaxGamepad - midMinGamepad) + midMinGamepad);
      } else {
          // Scale quickly in the negative outer range
          return (int)(percent * (minGamepad - midMinGamepad) + midMinGamepad);
      }
  }
}
