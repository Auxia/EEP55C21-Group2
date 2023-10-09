#include "HID-Project.h"

#define deadRangeLow 512-75
#define deadRangeHigh 512+75

const int pinButton = 2;  // Change to your button's pin
const int pinXAxis = A0;  // Change to your X-Axis pin
const int pinYAxis = A1;  // Change to your Y-Axis pin

void setup() {
  Serial.begin(9600);
  pinMode(pinButton, INPUT_PULLUP);

  // Sends a clean report to the host. This is important on any Arduino type.
  Gamepad.begin();
}

void loop() {
  // Read joystick and button values
  int16_t xAxis = readAxis(pinXAxis);
  int16_t yAxis = readAxis(pinYAxis);
  bool buttonPressed = !digitalRead(pinButton);

  // // Scale analog readings to gamepad range (-32767 to 32767)
  // xAxis = map(xAxis, 0, 1023, -32767, 32767);
  // yAxis = map(yAxis, 0, 1023, -32767, 32767);

  // Update gamepad state
  Gamepad.xAxis(xAxis);
  Gamepad.yAxis(yAxis);
  if (buttonPressed) {
    Gamepad.press(1);  // Press button 1
  } else {
    Gamepad.release(1);  // Release button 1
  }

  // Send updated gamepad state to host
  Gamepad.write();

  Serial.print("x = ");
  Serial.print(xAxis);
  Serial.print(", y = ");
  Serial.print(yAxis);
  Serial.println();
  
  // Add a small delay to make the sketch more responsive
  delay(10);
}

// readAxis with deadzone - deadzone keeps the values at 0 
int readAxis(byte pin) {
  int val = analogRead(pin);

  if (val < deadRangeLow)
    return map(val ,0, deadRangeLow, -32766, 0);

  if (val > deadRangeHigh)
    return map(val, deadRangeHigh, 1023, 0, 32767);

  return 0;
}
