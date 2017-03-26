// Stephen Flores
// Testing for the development of the CSComms library!!

// So the main functionality should be sending a character from the
// ground station to the container

// Secondary goal is telemetry transmission from the container to
// the ground station

// NOTE: Tutorials
// https://learn.sparkfun.com/tutorials/serial-communication
// https://learn.sparkfun.com/tutorials/exploring-xbees-and-xctu

// NOTE:
// Arduino sent values will show up in XCTU console
// Aruino received values will show up in Serial monitor

// NOTE: PINOUT
/* Teensy -> XBee
Pin 0 (RX) -> Dout (TX)
Pin 1 (TX) -> Din (RX)
Vin, Gnd

Where (looking top down, connector away)
Top left is 3.3v, then Dout, then Din. Ground is bottom-left pin.
*/

// NOTE: Module names and addresses:
/*
FlyBee:
    SH: 13A200
    SL: 41539F24
    ID: 1234
    DH: 0
    DL: 0
    FlyBeeContainer_profile.xml
GroundBee:
    SH: 13A200
    SL: 41539F1D
    ID: 1234
    DH: 0
    DL: 0
    GroundBee_profile.xml

*/

// TODO:
// Pin 14 is the voltage divider (analogread from it)
// Return voltage * 2 (must convert to voltage first)

// *** Required libraries *****************************************************
#include <SoftwareSerial.h>

// *** Objects ****************************************************************
SoftwareSerial xbee(0, 1); // Arduino RX, TX (XBee Dout, Din)

void setup() {
    delay(1000);
    Serial.begin(9600); // Necessary?
    xbee.begin(9600);
    pinMode(13, OUTPUT);

    Serial.println("Beginning test...");

    delay(2000);
}

long currentTime = 0;
long previousTime = 0;

void loop() {
    currentTime = millis();

    // Check for received command from ground. If so, send message.
    if (xbee.available()) {
        char c = xbee.read();
        Serial.println("Serial: Character received: " + c);
        xbee.println("XBee: Character received");
    }

    // Send a message to ground every second
    if (currentTime - previousTime > 1000) { // 2539 - 1400 = 1139 > 1000
        Serial.println("Printing thru xbee");
        float t = readVolt(14);
        xbee.println(t);
        previousTime = currentTime;
    }

}


// next things to try:
float readVolt(int pin) {
    double pinMillivolts = analogRead(pin) * (3300.0 / 1024.0);
    double volts = pinMillivolts / 1000.0;
    return volts * 2;
}
