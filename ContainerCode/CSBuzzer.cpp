// Stephen Flores
// CS Buzzer Implementation file

#include "Arduino.h"
#include "CSBuzzer.h"

CSBuzzer::CSBuzzer() {
    // Default constructor
    pin = 0;
    frequency = 440;
    on = false;
    pinMode(pin, OUTPUT);
}

CSBuzzer::CSBuzzer(int p) {
    pin = p;
    frequency = 440; // A = 440
    on = false;
    pinMode(p, OUTPUT);
}

void CSBuzzer::setFrequency(int f) {
    frequency = f;
}

void CSBuzzer::play() {
    // TODO: Play custom tone over time?

    // Assumes that play() is called every 1Hz
    if (on) {
        noTone(pin);
        on = false;
    } else {
        tone(pin, frequency);
        on = true;
    }
}
