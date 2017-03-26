// Stephen Flores
// Code for TMP36 temperature sensor


#ifndef _CSTemp_h
#define _CSTemp_h

#include "Arduino.h"


class CSTemp {
public:
    CSTemp(); // Default
    CSTemp(int p, int v); // Initialize
    float read();
private:
    int pin;
    int voltage;
};

#endif
