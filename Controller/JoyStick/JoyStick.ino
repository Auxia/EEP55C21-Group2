#include "HID-Project.h"

#define deadRangeLow 512-75
#define deadRangeHigh 512+75
#define NUM_READINGS 15  // for example, 10 readings for averaging

const int pinButton = 2;  // Change to your button's pin
const int pinXAxis = A0;  // Change to your X-Axis pin
const int pinYAxis = A1;  // Change to your Y-Axis pin

float exponent = 2;

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
  
  if (val >= deadRangeLow && val <= deadRangeHigh)
    return 0;  // Return 0 within the dead zone

  // Shift the reading so that the dead zone ends at 0
  int shiftedVal = (val < deadRangeLow) ? val - deadRangeLow : val - deadRangeHigh;
  
  // Apply exponential scaling
  int scaledVal = pow(abs(shiftedVal), exponent) * (shiftedVal > 0 ? 1 : -1);
  
  // Re-map the scaled value to the joystick range, considering the dead zone
  int outputRangeMin = (shiftedVal > 0) ? 0 : -32767;
  int outputRangeMax = (shiftedVal > 0) ? 32767 : 0;
  scaledVal = map(scaledVal, 0, pow(max(abs(deadRangeLow), abs(deadRangeHigh)), exponent), outputRangeMin, outputRangeMax);

  return scaledVal;
  
}
