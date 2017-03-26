// Test the nichrome on pin 11

#include "CSNichrome.h"

// *** Data and objects
int nichrome = 11;
int led = 13;
long currentTime = 0;

CSNichrome cutter; // Stack definition

void setup() {
    Serial.begin(9600);
    pinMode(nichrome, OUTPUT);
    pinMode(led, OUTPUT);

    cutter.init(11);
    Serial.println("Starting...");
    delay(3000);

    Serial.println("Cutting...");
    digitalWrite(nichrome, HIGH);
    digitalWrite(led, HIGH);
    delay(4000);
    digitalWrite(nichrome, LOW);
    digitalWrite(led, LOW);

}


int state = 0;
    int wait = 0;
    int cut = 1;
void loop() {
    // currentTime = millis();
    //
    // if (state == wait) {
    //     Serial.println("State 1");
    //     if (currentTime > 6000) {
    //         state = 1;
    //         cutter.start(currentTime, 4000);
    //         digitalWrite(led, HIGH);
    //     }
    // }
    //
    // if (state == cut) {
    //     Serial.print("State 2. Doing: ");
    //     Serial.println(cutter.hold(currentTime));
    //     if (currentTime > 20000) {
    //         state = 2; // breaks
    //         cutter.end();
    //         digitalWrite(led, LOW);
    //     }
    // }
    //
    // delay(300);
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
