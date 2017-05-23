// Stephen Flores
// Read from voltage divider on pin 14

#include "Arduino.h"

class CSVolt {
public:
    CSVolt();
    CSVolt(int p);
    float read();
private:
    int pin;
};

CSVolt::CSVolt() {
    Serial.println("CSVolt constructor");
    // Default
    pin = 0;
}

CSVolt::CSVolt(int p) {
    Serial.println("CSVolt constructor");
    pinMode(p, INPUT);
    pin = p;
}

float CSVolt::read() {
    Serial.println("CSVolt read");
    double pinMillivolts = analogRead(pin) * (3300.0 / 1024.0);
    double volts = pinMillivolts / 1000.0;
    return volts * 2;
}
