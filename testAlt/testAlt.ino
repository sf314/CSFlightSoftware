// Stephen Flores
// testAlt.ino
// Test the CSAlt altimeter code
// Implemented as library
// Now includes cutting code and whatnot

// *** Required libs:
//#include "IntersemaBaro.h"
#include "Wire.h"
#include "CSAlt.h"

int nichrome = 11;
int led = 13;

// *** Objects:
CSAlt baro;

void setup() {
    Wire.begin(); // 需要吗？
    Serial.begin(9600);
    pinMode(nichrome, OUTPUT);
    pinMode(led, OUTPUT);

    baro.init();
    baro.setGroundHeight(baro.alt());
    delay(1000);
}

int state = 0;
    int ascending = 0;
    int descending = 1;

float vel = 0;
float prevAlt = 0;
float currentAlt = 0;

void loop() {
    Serial.println("State: " + String(state));

    float currentAlt = baro.alt();
    Serial.println(currentAlt);
    Serial.println(baro.altRadar());

    vel = currentAlt - prevAlt;

    if (state == 0) {
        Serial.println("\tAscending");
        if (vel < 0) {
            state = descending;
        }
    }

    if (state == 1) {
        if (currentAlt < 350) {
            cutNichrome(6);
            delay(10000);
        }
    }

    prevAlt = currentAlt;
    delay(1000);
}


// *** Funcs
void cutNichrome(int seconds) {
    // Start cut
    digitalWrite(nichrome, HIGH);

    for (int i = 0; i < seconds; i++) {
        digitalWrite(led, HIGH); delay(500);
        digitalWrite(led, LOW); delay(500);
    }

    // End cut
    digitalWrite(nichrome, LOW);
}
