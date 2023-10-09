#include "HID-Project.h"

const int pinButton = 2;  // Change to your button's pin
const int pinXAxis = A0;  // Change to your X-Axis pin
const int pinYAxis = A1;  // Change to your Y-Axis pin
const int dialPin1 = 9; // Labeled "1" on encoder
const int dialPin2 = 10; // Labeled "2" on encoder
const int dialPin3 = 11; // Labeled "4" on encoder
const int dialPin4 = 12; // Labeled "8" on encoder

void setup() {
  delay(1000);
  Serial.begin(9600);
  pinMode(pinButton, INPUT_PULLUP);
  pinMode(dialPin1, INPUT_PULLUP); 
  pinMode(dialPin2, INPUT_PULLUP);
  pinMode(dialPin3, INPUT_PULLUP);
  pinMode(dialPin4, INPUT_PULLUP);

  // Sends a clean report to the host. This is important on any Arduino type.
  Gamepad.begin();
}

void loop() {
  // Read joystick and button values
  int16_t xAxis = analogRead(pinXAxis);
  int16_t yAxis = analogRead(pinYAxis);
  int8_t zAxis = 0;
  bool buttonPressed = !digitalRead(pinButton);

  // Read grey code encoder 
  int dialVal1 = digitalRead(dialPin1);
  int dialVal2 = digitalRead(dialPin2);
  int dialVal3 = digitalRead(dialPin3);
  int dialVal4 = digitalRead(dialPin4);
  zAxis = 8*(1-dialVal4) + 4*(1-dialVal3) + 2*(1-dialVal2) + 1-dialVal1; // thrust

  // Scale analog readings to gamepad range (-32767 to 32767)
  xAxis = map(xAxis, 0, 1023, -32767, 32767);
  yAxis = map(yAxis, 0, 1023, -32767, 32767);
  zAxis = map(zAxis, 0, 15, 0, 127);

  // Update gamepad state
  Gamepad.xAxis(xAxis);
  Gamepad.yAxis(yAxis);
  Gamepad.zAxis(zAxis);
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
  Serial.print(", z (thrust) = ");
  Serial.print(zAxis);
  Serial.println();
  
  // Add a small delay to make the sketch more responsive
  delay(10);
}
