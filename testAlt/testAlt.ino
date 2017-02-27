// Stephen Flores
// testAlt.ino
// Test the CSAlt altimeter code
// Implemented as library

// *** Required libs:
#include "IntersemaBaro.h"
#include "Wire.h"
#include "CSAlt.h"

// *** Objects:
CSAlt baro;

void setup() {
    Serial.begin(9600);
    baro.init();
    baro.setGroundHeight(baro.alt());
    delay(1000);
}

void loop() {
    Serial.println(baro.alt());
    delay(1000);
}
