// For the sparkfun altimeter

#include <Wire.h>
#include "SparkFunMPL3115A2.h"

// *** Objects
MPL3115A2 alt;

void setup() {
    Wire.begin();
    Serial.begin(9600);

    alt.begin();
    alt.setModeAltimeter();
    alt.setOversampleRate(7); // Needed
    alt.enableEventFlags(); // Also needed
}

void main() {
    float altitude = alt.readAltitude();
    Serial.println("Alt: " + String(altitude));

    float temperature = alt.readTemp();
    Serial.println("Temp: " + String(temperature));

    Serial.println();
}
