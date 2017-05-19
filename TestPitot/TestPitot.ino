// Test pitot tube

// *** Pinout:
// With tubes pointing up, and pins facing away:
/*
           /\ /\
GND     C|1----8|D
VIN     C|2    7|D
SDA     C|3    6|D
SCL     C|4____5|D    INT/SS

Place 100nF (#104) capacitor connecting V and Gnd.
*/

#include <Wire.h>
#include <SPI.h>
#include "CSPitot.h"

void setup() {
    Serial.begin(9600);
    Wire.begin();
    delay(2000);
    Serial.println("Testing Pitot Tube");
}

void loop() {
    Serial.println("Loop");
    Serial.println(CSPitot_getVelocity());
    delay(1000);
}
