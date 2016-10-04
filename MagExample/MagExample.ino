// Magnetometer test
// I2C
// Don't forget address, begin, etc.

#include <Wire.h>
#include <CSMag.h>

CSMag mag = CSMag();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(200);
  pinMode(13, OUTPUT);
  Serial.println("A");
  mag.config();
  Serial.println("Hello, there!");
  delay(1000);
}

void loop() {
  
  Serial.print("x = ");
  Serial.print(mag.readx());
  
  Serial.print("\ty = ");
  Serial.print(mag.ready());
  Serial.write("Hello");

  Serial.print("\tz = ");
  Serial.println(mag.readz());

  delay(250);
}



