
#include "HID-Project.h"

#define offsetX 0
#define offsetY 0
#define deadRangeLow 512-75
#define deadRangeHigh 512+75
#define deadRangeLow2 512-500
#define deadRangeHigh2 512+500
#define outputRange 5000
#define NUM_READINGS 15  // for example, 10 readings for averaging
#define zThreshold 11
#define thrustOutput 100

// For these values the Potentiometer will perform assisted thrust
#define ASSISTED_THRUST_LOW 3
#define ASSISTED_THRUST_HIGH 4


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

  bool buttonPressed = !digitalRead(pinButton);

  int8_t thrust = readThrust();

  // Update gamepad state
  Gamepad.xAxis(averageX);
  Gamepad.yAxis(averageY);
  Gamepad.zAxis(thrust);
  // if (thrust == 1) {
  //   Gamepad.press(1);  // Press button 1
  // } else {
  //   Gamepad.release(1);  // Release button 1
  // }

  // Send updated gamepad state to host
  Gamepad.write();

  Serial.print("x = ");
  Serial.print(averageX);
  Serial.print(", y = ");
  Serial.print(averageY);
  Serial.print(", z (thrust) = ");
  Serial.print(thrust);
  Serial.println();
  
  // Add a small delay to make the sketch more responsive
  delay(10);
}

int trim(int number) {
  if (number < -32766) {
    return -32766;
  }
  if (number > 32767) {
    return 32767;
  }
  return number;
}

int getOffset(byte pin) {
  return pin == pinXAxis ? offsetX : offsetY; 
}

// readAxis with deadzone - deadzone keeps the values at 0 
int readAxis(byte pin) {
  int val = analogRead(pin);

  if (val < deadRangeLow2) 
  {
    return trim(map(val, 0, deadRangeLow2, -32766, -outputRange) + getOffset(pin));
  }

  if (val < deadRangeLow)
  {
    // return map(val ,0, deadRangeLow, -32766, 0);
    return trim(map(val, deadRangeLow2, deadRangeLow, -outputRange, 0) + getOffset(pin));
  }

  if (val < deadRangeHigh)
  {
    return getOffset(pin);
  }

  if (val < deadRangeHigh2) {
    return trim(map(val, deadRangeHigh, deadRangeHigh2, 0, outputRange) + getOffset(pin));
  }

  else {
    return trim(map(val, deadRangeHigh2, 1023, outputRange, 32767) + getOffset(pin));
  }
}

int readThrust() {
  // Read grey code encoder 
  int dialVal1 = digitalRead(dialPin1);
  int dialVal2 = digitalRead(dialPin2);
  int dialVal3 = digitalRead(dialPin3);
  int dialVal4 = digitalRead(dialPin4);
  int zAxis = 8*(1-dialVal4) + 4*(1-dialVal3) + 2*(1-dialVal2) + 1-dialVal1; // thrust

  Serial.print("zAxis = ");
  Serial.print(zAxis);
  
  if(zAxis == 0){
    
    // Edge case-> does not register 0 as non assisted control when returning from assisted control
    Gamepad.release(1); 
    return 0;
  }

  if (zAxis < ASSISTED_THRUST_LOW) {
    // Same as previous case. We can not let the assisted mode come in place here
    Gamepad.release(1); 
    return map(zAxis, 1, ASSISTED_THRUST_LOW, 30, thrustOutput);
  }

  // Check for assisted thrust condition
  if (zAxis >= ASSISTED_THRUST_LOW && zAxis <= ASSISTED_THRUST_HIGH) {
    Gamepad.press(1);  // Press button 1
    return 1;  // Assisted thrust value
  } else {
    Gamepad.release(1);  // Release button 1
  }

  if (zAxis < zThreshold) {
    return map(zAxis, ASSISTED_THRUST_HIGH, zThreshold, 70, thrustOutput);
  }
  else {
    return map(zAxis, zThreshold, 15, thrustOutput, 127);
  }
}