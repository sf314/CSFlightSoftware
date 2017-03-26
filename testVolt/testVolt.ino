// Stephen Flores
// Test voltage divider on pin 14 (analog)

#include <SoftwareSerial.h>

SoftwareSerial xbee(0, 1);

int voltPin = 14;

float arr[] = {0, 0, 0, 0, 0};

void setup() {
    Serial.begin(9600);
    pinMode(voltPin, INPUT);
}

long currentTime = 0;
long previousTime = 0;
bool on = false;

void loop() {
    currentTime = millis();

    //Serial.println(readVolt(voltPin));
    //Serial.println(analogRead(14));

    if (currentTime - previousTime > 500) {
        addVolt(voltPin);
        float v = average();
        Serial.println(v);

        if (xbee.available()) {
            char c = xbee.read();
            Serial.println("Serial: Character received: " + c);
            xbee.println("XBee: Character received");
        }

        xbee.println("hey");

        if (on) {
            digitalWrite(13, LOW);
            on = false;
        } else {
            digitalWrite(13, HIGH);
            on = true;
        }

        previousTime = currentTime;
    }

}


float readVolt(int pin) {
    double pinMillivolts = analogRead(pin) * (3300.0 / 1024.0);
    double volts = pinMillivolts / 1000.0;
    return volts * 2;
}

void addVolt(int pin) {
    double pinMillivolts = analogRead(pin) * (3300.0 / 1024.0);
    double volts = pinMillivolts / 1000.0;
    float value = volts * 2; // Same as readVolt();

    for (int i = 0; i < 4; i++) {
        arr[i] = arr[i + 1];
    }

    arr[4] = value;
    Serial.print("Read: " + String(value) + "\tAverage: ");
}

float average() {
    float sum = 0.0;

    for (int i = 0; i < 5; i++) {
        sum = sum + arr[i];
    }

    return sum / 5.0;
}
