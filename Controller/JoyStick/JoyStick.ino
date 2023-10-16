
#include "HID-Project.h"

#define deadRangeLow 512-75
#define deadRangeHigh 512+75
#define deadRangeLow2 512-300
#define deadRangeHigh2 512+300
#define NUM_READINGS 30  // for example, 10 readings for averaging

const int pinButton = 2;  // Change to your button's pin
const int pinXAxis = A0;  // Change to your X-Axis pin
const int pinYAxis = A1;  // Change to your Y-Axis pin
const int dialPin1 = 9; // Labeled "1" on encoder
const int dialPin2 = 10; // Labeled "2" on encoder
const int dialPin3 = 11; // Labeled "4" on encoder
const int dialPin4 = 12; // Labeled "8" on encoder

int readingsX[NUM_READINGS];  // the readings from the analog input
int readingsY[NUM_READINGS];  // the readings from the analog input
int readIndex = 0;  // the index of the current reading
int totalX = 0;  // the running total
int totalY = 0;  // the running total
int averageX = 0;  // the average
int averageY = 0;  // the average

void setup() {
  for (int thisReading = 0; thisReading < NUM_READINGS; thisReading++) {
    readingsX[thisReading] = 0;
    readingsY[thisReading] = 0;
  }

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
  // subtract the last reading:
  totalX = totalX - readingsX[readIndex];
  totalY = totalY - readingsY[readIndex];
  // read from the sensor:
  readingsX[readIndex] = readAxis(pinXAxis);
  readingsY[readIndex] = readAxis(pinYAxis);
  // add the reading to the total:
  totalX = totalX + readingsX[readIndex];
  totalY = totalY + readingsY[readIndex];
  // advance to the next position in the array:
  readIndex = readIndex + 1;
  // if we're at the end of the array...
  if (readIndex >= NUM_READINGS) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }
  // calculate the average:
  averageX = totalX / NUM_READINGS;
  averageY = totalY / NUM_READINGS;

  int8_t zAxis = 0;
  bool buttonPressed = !digitalRead(pinButton);

  // Read grey code encoder 
  int dialVal1 = digitalRead(dialPin1);
  int dialVal2 = digitalRead(dialPin2);
  int dialVal3 = digitalRead(dialPin3);
  int dialVal4 = digitalRead(dialPin4);
  zAxis = 8*(1-dialVal4) + 4*(1-dialVal3) + 2*(1-dialVal2) + 1-dialVal1; // thrust

  // Scale analog readings to gamepad range (-32767 to 32767)
  // xAxis = map(xAxis, 0, 1023, -32767, 32767);
  // yAxis = map(yAxis, 0, 1023, -32767, 32767);
  zAxis = map(zAxis, 0, 15, 0, 127);

  // Update gamepad state
  Gamepad.xAxis(averageX);
  Gamepad.yAxis(averageY);
  Gamepad.zAxis(zAxis);
  if (buttonPressed) {
    Gamepad.press(1);  // Press button 1
  } else {
    Gamepad.release(1);  // Release button 1
  }

  // Send updated gamepad state to host
  Gamepad.write();

  Serial.print("x = ");
  Serial.print(averageX);
  Serial.print(", y = ");
  Serial.print(averageY);
  Serial.print(", z (thrust) = ");
  Serial.print(zAxis);
  Serial.println();
  
  // Add a small delay to make the sketch more responsive
  delay(10);
}

// readAxis with deadzone - deadzone keeps the values at 0 
int readAxis(byte pin) {
  int val = analogRead(pin);

  if (val < deadRangeLow2) 
  {
    return map(val, 0, deadRangeLow2, -32766, -8000);
  }

  if (val < deadRangeLow)
  {
    // return map(val ,0, deadRangeLow, -32766, 0);
    return map(val, deadRangeLow2, deadRangeLow, -8000, 0);
  }

  if (val < deadRangeHigh)
  {
    return 0;
  }

  if (val < deadRangeHigh2) {
    return map(val, deadRangeHigh, deadRangeHigh2, 0, 8000);
  }

  else {
    return map(val, deadRangeHigh2, 1023, 8000, 32767);
  }
}
