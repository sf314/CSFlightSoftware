// Stephen Flores
// Fake the telemetry structure of CanSat
// Both Container and Payload (so parsing is required)

#include <SoftwareSerial.h>
SoftwareSerial xbee(0, 1);

// Changing telem:
int packetCount = 0;
bool led = false;

void setup() {
    Serial.begin(9600);
    xbee.begin(9600);
    pinMode(13, OUTPUT);
    pinMode(6, OUTPUT);
}

long currentTimeG = 0;
long previousTimeG = 0;

long currentTimeC = 0;
long previousTimeC = 0;

String dat = "";

void loop() {

    currentTimeG = millis();
    currentTimeC = currentTimeG + 500;

    if (currentTimeG - previousTimeG > 1000) {
        if (led) {
            digitalWrite(13, LOW);
            led = false;
            digitalWrite(6, LOW);
        } else {
            digitalWrite(13, HIGH);
            led = true;
            digitalWrite(12, HIGH);
        }
        float t = currentTimeG / 10000.0;
        dat = "2848,GLIDER,";
        dat += String(t * 10) + ",";
        dat += String(packetCount) + ",";
        dat += String(225 * sin(t) + 225) + ","; // alt
        dat += String(20 * sin(t) + 1000) + ","; // press
        dat += String(4 * sin(t) + 15) + ","; // speed
        dat += String(5 * sin(t) + 25) + ","; // temp
        dat += String(3.3 * sin(t) + 3.3) + ","; // volt
        dat += String(180 * sin(t) + 180) + ","; // heading
        dat += "2"; // state
        Serial.println(dat);
        xbee.println(dat);

        previousTimeG = currentTimeG;
    }

    if (currentTimeC - previousTimeC > 1000) {
        float t = currentTimeC / 10000.0;
        dat = "2848,CONTAINER,";
        dat += String(t * 10) + ",";
        dat += String(packetCount) + ",";
        dat += String(230 * sin(t) + 230) + ","; // alt
        dat += String(5 * sin(t) + 25) + ","; // temp
        dat += String(0.5 * sin(t) + 5.5) + ","; // volt
        dat += "1"; // state
        Serial.println(dat);
        xbee.println(dat);

        packetCount++;
        previousTimeC = currentTimeC;
    }
}
