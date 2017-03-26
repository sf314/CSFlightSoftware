


void setup() {
    Serial.begin(9600);
    pinMode(13, OUTPUT);
}

long currentTime = 0;
long previousTime = 0;
bool on = false;

void loop() {
    currentTime = millis();

    if (currentTime - previousTime > 1000) {
        if (on) {
            digitalWrite(13, LOW);
            Serial.println("on");
            on = false;
        } else {
            Serial.println("off");
            digitalWrite(13, HIGH);
            on = true;
        }
        previousTime = currentTime;
    }

}
