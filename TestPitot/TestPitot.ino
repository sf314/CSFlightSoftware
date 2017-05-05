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

Runs in the following conditions:
Addr: 0x28 for MS4525DO
5V logic arduino: (OSEPP Uno)
    5V supply and 3V supply

Does not run in following:
3.3V logic arduino: (Teensy)
    3.3 supply
    Will hang on Wire.endTransmission()
    Return code 4 if it gets past that
*/

#include <Wire.h>
#include <SPI.h>
#include <math.h>
#include "CSPitot.h"
#include "CSimu.h"
#define pi 3.14159265358

CSimu imu;

void setup() {
    Serial.begin(9600);
    Wire.begin();
    SPI.begin();
    delay(2000);
    Serial.println("Testing Pitot Tube");

    imu.config();
}

void loop() {
    Serial.println("Loop");
    //Serial.println(CSPitot_getVelocity());
    imu.updateSensors();

    float x = imu.mag.x;
    float y = imu.mag.y;
    Serial.println(headingFromIMU(x, y));
    delay(1000);
}





// ********** Calculate Heading based on Hx, Hy (adafruit) ********************
// May need to change axes in order to calc it right
// Assumes that X direction is prograde
    // If Y is prograde, then feed Y as X, and -X as Y
    // Should be fine based on schematics
    // If backwards (-X is prograde), feed -X as X and -Y as Y
int headingFromIMU(float x, float y) {
    float returnValue = 0.0;
    if (y > 0) {
        returnValue = (int)(90 - (atan(x / y)) * ( 180 / pi));
    } else if (y < 0) {
        returnValue = (int)(270 - (atan(x / y)) * (180 / pi));
    } else if (y == 0 && x < 0) {
        returnValue = 180;
    } else if (y == 0 && x > 0) {
        returnValue = 0;
    }

    return (int)(round(returnValue));
}
