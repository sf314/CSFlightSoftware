// Cansat 2017 Container software
// Must access CSAlt, CSComms, and CSLog
// Requirements:
// Transmit telemetry at 1Hz (rq. fields in req. format)
// Deploy payload (via payload release function) at 400m radar alt
// Activate beacon and stop telemetry 2s after deploying payload
// Telemetry complete!

// NOTE: ***** PINOUT *****
/*
Altimeter:
    SCL -> Pin 19 (SCL)
    SDA -> Pin 18 (SDA)
Temp sensor:
    Data -> Pin 22 (A8)
Nichrome:
    Cut -> Pin 11
XBee:
    TX/DOUT -> Pin 0 (RX)
    RX/DIN -> Pin 1 (TX)
Buzzer:
    Vin -> Pin 6 (PWM)
*/

// NOTE: *** EEPROM addresses ***
/*
altitude @ 0 (stored as int, rounds to nearest)
state at 1
packet count at 2
time at 3
*/

// NOTE:
// Container does not need SD!
// Need to include temp sensor on payload
// Stop telemetry 2s after deployment
// Currently, mission time is not persisted. Reboots reset it to zero.

// TODO:
// Stop telemetry 2 seconds after deployment


// ********** Required Libraries **********************************************
#include "Wire.h"
#include "IntersemaBaro.h"
#include "CSAlt.h"
#include "CSTemp.h"
#include "CSBuzzer.h"
#include "CSVolt.h"
#include <EEPROM.h>
#include "SoftwareSerial.h"

// ********** Objects *********************************************************
CSAlt baro;
CSTemp temp;
CSBuzzer buzzer;
CSVolt volt;
SoftwareSerial xbee(0, 1);

int led = 13;
int nichromePin = 11;
bool on = false;
String dataString = "";
float tempe = 344.0; // Custom launchpads here if desired

// ********** Control variables ***********************************************
int state                 = 0;
    const int boot        = 0;
    const int launchpad   = 1;
    const int ascent      = 2;
    const int descent     = 3;
    const int deploy      = 4;

// ********** Time variables **************************************************
long currentTime    = 0;
long previousTime   = 0;

long  startTime     = 0;
long restoredTime   = 0;

#define refreshRate   1000

long ledPrevTime    = 0;
int ledBlinkRate    = 1000;

// ********** Telemetry structure *********************************************
float currentAlt    = 0;
float previousAlt   = 0;
float verticalSpeed = 0;
float currentTemp   = 0;
float currentVolt   = 0;
int packetCount     = 1;

// ********** State change conditions *****************************************
#define targetAltitude          300
#define ascentVSpeedThreshold   1
#define ascentAltThreshold      10
#define descentVSpeedThreshold  -1
#define forceDeployThreshold    200
#define noVSpeedThreshold       1
#define cutTime                 4 // seconds
#define cutoffAlt               100

// ********** Setup Function *********************************************#####
void setup() {
    // Startup procedure
    Serial.begin(9600);
    Wire.begin();
    pinMode(led, OUTPUT);
    delay(1000);

    // Restore values from reboot
    restore();

    // * Barometer
    Serial.println("Initializing barometer");
    baro.init();
    currentAlt = baro.altRadar(); // Initialize alt stuff (GH restored)
    previousAlt = currentAlt;

    // * Temperature
    temp = CSTemp(22, 3300);

    // * Nichrome wire
    pinMode(nichromePin, OUTPUT);

    // * Buzzer
    buzzer = CSBuzzer(6);
    buzzer.setFrequency(800);

    // * Voltage divider
    volt = CSVolt(14);

    // * CSComms
    CSComms_begin(9600);
    CSComms_log("Reboot");

    // * Time
    Serial.println("Setting start time");
    startTime = setStartTime();
}

// ********** Loop Function **********************************************#####
void loop() {
    currentTime = restoredTime + millis() - startTime;

    // *** Non-time triggered functions. Run every time. **********************

    // * Receive command
    if (CSComms_available()) {
        char c = CSComms_read();
        Serial.println("Received: " + String(c));

        CSComms_parse(c);
    }

    // *** Time-triggered functions. Must happen on every iteration ***********
    if (currentTime - previousTime > refreshRate) {
        // Switch on state
        switch (state) {
            case 0:
                boot_f(); break;
            case 1:
                launchpad_f(); break;
            case 2:
                ascent_f(); break;
            case 3:
                descent_f(); break;
            case 4:
                deploy_f(); break;
            default:
                Serial.println(F("Error: invalid state"));
                state = 0;
        }

        // * Print data
        Serial.println("\nT: " + String(currentTime) + ",\tState: " + String(state) + ",\nAlt: " + String(currentAlt) + ",\tvSpeed: " + String(verticalSpeed) + ",\tTemp: " + String(currentTemp));

        // Set previous variables
        previousTime = currentTime;
        previousAlt = currentAlt;
    }

    // * Blink the led for [blink rate]
    if (currentTime - ledPrevTime > ledBlinkRate) {
        if (on) {
            digitalWrite(led, LOW); on = false;
        } else {
            digitalWrite(led, HIGH); on = true;
        }
        ledPrevTime = currentTime;
    }

}






// ********** Miscellaneous functions *****************************************
long setStartTime() {
    // TODO: Recover time t from file if it exists
    return millis(); // do something smarter(?) eventually
}



// ********** State Functions *************************************************
void boot_f() {
    // Restore data from EEPROM.
    // Done in setup already?
    //restore();
    CSComms_log("Boot success");
    state = launchpad;
}

void launchpad_f() {
    updateTelemetry();
    transmitTelemetry();

    if (currentAlt > ascentAltThreshold) {
        state = ascent;
        delay(1000); // Don't let noise ruin it
    }
}

void ascent_f() {
    updateTelemetry();
    transmitTelemetry();

    if (verticalSpeed < descentVSpeedThreshold) {
        state = descent;
    }
}

void descent_f() {
    updateTelemetry();
    transmitTelemetry();

    if (currentAlt < targetAltitude) {
        // Start cutting sequence
        startCut();

        // Wait n-seconds, keep transmitting telemetry
        for (int i = 0; i < cutTime; i++) {
            updateTelemetry();
            transmitTelemetry();
            delay(1000);
        }
        stopCut();

        state = deploy;
    }
}

void deploy_f() {
    updateTelemetry();
    transmitTelemetry();

    // Final burn (but don't burn the field)
    if (currentAlt < forceDeployThreshold && currentAlt > cutoffAlt) {
        // Start cutting sequence
        startCut();

        // Wait n-seconds, keep transmitting telemetry
        for (int i = 0; i < (cutTime / 2); i++) {
            updateTelemetry();
            transmitTelemetry();
            delay(1000);
        }
        stopCut();
    }

    // Play buzzer
    buzzer.play();
}



// ********** Implement CSComms functions *************************************
void CSComms_add(long l) {
    dataString += String(l) + ", ";
}

void CSComms_add(float f) {
    dataString += String(f) + ", ";
}

void CSComms_add(String s) {
    dataString += s;
}

void CSComms_log(String s) {
    xbee.println(s);
}

void CSComms_transmit() {
    xbee.println(dataString);
    dataString = "";
    packetCount++;
}

bool CSComms_available() {
    return xbee.available();
}

char CSComms_read() {
    return xbee.read();
}

void CSComms_begin(int baud) {
    xbee.begin(baud);
}

void CSComms_parse(char c) {
    switch (c) {
        case 'x': // Auto cut command
            //nichrome.start(currentTime, cutTime);
            digitalWrite(11, HIGH);
            delay(6000);
            digitalWrite(11, LOW);
            //ledBlinkRate = 250;
            state = deploy;
            break;
        case '0': // switch to state 0 (reboot)
            state = boot; break;
        case '1': // switch to launchpad
            state = launchpad; break;
        case '2': // switch to ascent
            state = ascent; break;
        case '3': //  switch to descent
            state = descent; break;
        case '4': // switch to deploy
            state = deploy; break;
        case 'g': // Set ground height from current (use on launchpad) (and persist it)
            baro.setGroundHeight(baro.alt());
            currentAlt = baro.altRadar();
            previousAlt = currentAlt;
            store();
            Serial.println("Saving altitude: " + String(currentAlt));
            break;
        case 'c': // Reset packet count to 1 and persist it
            packetCount = 1;
            store();
            break;
        case 'p': // Print current persisted info
            CSComms_log("St: " + String(state) + ", PC: " + String(packetCount) + ", GH: " + String(baro.getGroundHeight()));
            break;
        case 'x': // Force deploy mode
            state = deploy;
            break;
        default:
            Serial.println("Invalid command char");
    }
}




// ********** Implement data update and transmit functions ********************
void updateTelemetry() {
    currentAlt = baro.altRadar();
    verticalSpeed = (currentAlt - previousAlt) / ((currentTime - previousTime) / 1000); // m/s
    currentTemp = temp.read();
    currentVolt = volt.read();
}

void transmitTelemetry() {
    CSComms_add("2848, CONTAINER, ");
    CSComms_add(((float)currentTime / 1000.0));
    CSComms_add((long)packetCount);
    CSComms_add(currentAlt);
    CSComms_add(currentTemp);
    CSComms_add(currentVolt);
    CSComms_add((long)state);
    CSComms_transmit();
}



// ********** Cut nichrome functions ******************************************
void startCut() {
    digitalWrite(nichromePin, HIGH);
}

void stopCut() {
    digitalWrite(nichromePin, LOW);
}




// ********** EEPROM Persistence functions ************************************
void store() {
    CSComms_log("Storing to EEPROM");
    int addr = 0;
    EEPROM.write(addr, state);
    addr += sizeof(int);
    EEPROM.write(addr, packetCount);
    addr += sizeof(int);
    EEPROM.write(addr, (int)baro.getGroundHeight());
    addr += sizeof(int);
    EEPROM.write(addr, currentTime); // Type works?
}

void restore() {
    CSComms_log("Restoring from EEPROM");
    int addr = 0;
    state = EEPROM.read(addr);
    addr += sizeof(int);
    packetCount = EEPROM.read(addr);
    addr += sizeof(int);
    baro.setGroundHeight((float)EEPROM.read(addr));
    addr += sizeof(int);
    restoredTime = EEPROM.read(addr);
}
