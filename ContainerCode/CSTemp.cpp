// Stephen Flores
// Implementation of CSTemp for TMP36

#include "CSTemp.h"

CSTemp::CSTemp() {
    // Default constructor
    pin = 0;
    voltage = 3300;
    pinMode(0, INPUT);
}

CSTemp::CSTemp(int p, int v) {
    pin = p;
    voltage = v;
    pinMode(p, INPUT);
}

float CSTemp::read() {
    double pinMillivolts = analogRead(pin) * ((double)voltage / 1024);
    double celsius = (pinMillivolts - 500) / 10; // in celsius

    //double fahrenheit =  celsius * 1.8 + 32;
    return celsius;
}
