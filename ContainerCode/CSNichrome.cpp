// Implementation file for CSNichrome

#include <Arduino.h>
#include "CSNichrome.h"

// Initialize data variables and set nichrome pin as OUTPUT
void CSNichrome::init(int p) {
    pin = p;
    pinMode(pin, OUTPUT);

    duration = 0;
    startTime = 0;
}

// Set variables and begin cutting
void CSNichrome::start(long st, int dur) {
    //this->startTime = startTime; // Can use this reference for setting
    startTime = st;
    duration = dur;
    digitalWrite(pin, HIGH);
}

// Keep cutting until time runs out
void CSNichrome::hold(long t) {
    if (t < startTime + duration) {
        digitalWrite(pin, HIGH);
    } else {
        digitalWrite(pin, LOW);
    }
}

// Terminate cutting process
void CSNichrome::end() {
    duration = 0;
    digitalWrite(pin, LOW);
}
