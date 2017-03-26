// Cansat 2017 Container software
// Must access CSAlt, CSComms, and CSLog
// Requirements:
// Transmit telemetry at 1Hz (rq. fields in req. format)
// Deploy payload (via payload release function) at 400m radar alt
// Activate beacon and stop telemetry 2s after deploying payload

// NOTE: ***** PINOUT *****
/*
Altimeter:
    SCL -> Pin 19 (SCL)
    SDA -> Pin 18 (SDA)
Temp sensor:
    Data -> Pin 22 (A8)
Nichrome:
    Cut -> Pin 11 // Should be changed for SD card!
XBee:
    TX/DOUT -> Pin 0 (RX)
    RX/DIN -> Pin 1 (TX)
Buzzer:
    Vin -> Pin 6 (needs PWM)
*/

// NOTE:
// Container does not need SD!
// Need to include temp sensor on payload
// Stop telemetry 2s after deployment

// TODO:
// Stop telemetry 2 seconds after deployment
// Put getting data and sending telemetry into discrete functions, and call them
//  from their functions

// ********** Required Libraries **********************************************
#include "Wire.h"
#include "IntersemaBaro.h"
#include "CSAlt.h"
#include "CSNichrome.h"
#include "CSTemp.h"
#include "CSBuzzer.h"
#include "CSVolt.h"
#include "SoftwareSerial.h"
//#include "CSComms.h" // pins must be edited in .h

// ********** Objects *********************************************************
CSAlt baro;
CSNichrome nichrome;
CSTemp temp;
CSBuzzer buzzer;
CSVolt volt;
SoftwareSerial xbee(0, 1);

int led = 13;
bool on = false;
float heightOfLaunchpad = 0;
String dataString = "";

// ********** Control variables ***********************************************
int state             = 0;
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
#define ascentAltThreshold      50
#define descentVSpeedThreshold  -1
#define forceDeployThreshold    200
#define noVSpeedThreshold       1
#define cutTime                 6000
#define cutoffAlt               100

// ********** Setup Function *********************************************#####
void setup() {
    // Startup procedure
    Serial.begin(9600);
    Wire.begin();
    pinMode(led, OUTPUT);
    delay(1000);

    // * Barometer
    Serial.println(F("Initializing barometer"));
    baro.init();
    baro.setGroundHeight(baro.alt()); // set startup ground height as zero.
    //baro.setGroundHeight(heightOfLaunchpad); // Set custom if known
    currentAlt = baro.alt(); // Initialize alt stuff
    previousAlt = currentAlt;

    // * Temperature
    temp = CSTemp(22, 3300);

    // * Nichrome wire
    nichrome.init(11); // Set nichrome pin (digital)

    // * Buzzer
    buzzer = CSBuzzer(6);
    buzzer.setFrequency(800);

    // * Voltage divider
    volt = CSVolt(14);

    // * CSComms
    CSComms_begin(9600);

    // * Time
    Serial.println(F("Setting start time"));
    // Restore state and packet count if from reset (and log to local telem)
    startTime = setStartTime();
}

// ********** Loop Function **********************************************#####
void loop() {
    currentTime = millis() - startTime;

    // *** Non-time triggered functions. Run every time. ********************

    // * Receive command
    if (CSComms_available()) {
        char c = CSComms_read();
        Serial.println("Received: " + String(c));

        // Switch on received command char
        switch (c) {
            case 'x': // Auto cut command
                //nichrome.start(currentTime, cutTime);
                digitalWrite(11, HIGH);
                delay(6000);
                digitalWrite(11, LOW);
                //ledBlinkRate = 250;
                state = deploy;
                break;
            default:
                Serial.println("Invalid command char");
        }
    }

    // *** Time-triggered functions. Must happen on every iteration *********
    if (currentTime - previousTime > refreshRate) {
        // * Get most recent data
        currentAlt = baro.altRadar();
        verticalSpeed = (currentAlt - previousAlt) / ((currentTime - previousTime) / 1000); // m/s
            // May be inf at first cuz div by zero
        currentTemp = temp.read();
        currentVolt = volt.read();


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

        // * Save data
        CSComms_add("2848, CONTAINER, ");
        CSComms_add(((float)currentTime / 1000.0));
        CSComms_add((long)packetCount);
        CSComms_add(currentAlt);
        CSComms_add(currentTemp);
        CSComms_add(currentVolt);
        CSComms_add((long)state);
        CSComms_transmit();

        // * Print data
        Serial.println("\nT: " + String(currentTime) + ",\tState: " + String(state) + ",\nAlt: " + String(currentAlt) + ",\tvSpeed: " + String(verticalSpeed) + ",\tTemp: " + String(currentTemp));

        // Set previous variables
        previousTime = currentTime;
        previousAlt = currentAlt;
    }

    // * Blink the led for [blink rate]
    if (currentTime - ledPrevTime > ledBlinkRate) {
        if (on) {
            digitalWrite(led, LOW);
            on = false;
        } else {
            digitalWrite(led, HIGH);
            on = true;
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
    // TODO: Recover from restart: reset to prior state.

    // Check data to see what state you should be in!
    // If ground altitude is set, then you can still use radar alt.
    float a1 = baro.altRadar();
    delay(1000);
    float a2 = baro.altRadar();

    if (a2 - a1 > ascentVSpeedThreshold) {  // going up
        state = ascent;
    } else {                                // going down
        if (a1 > targetAltitude) {              // pre-deployment
            state = descent;
        } else {                                // post-deployment
            state = deploy;
        }
    }

    // Maybe not moving: must be on launchpad
    if (abs(a2 - a1) < noVSpeedThreshold) {
        state = launchpad;
    }
}

void launchpad_f() {
    // Wait for cue to  begin ascent.
    // if (verticalSpeed > ascentVSpeedThreshold) {
    //     state = ascent;
    // }

    if (currentAlt > ascentAltThreshold) {
        state = ascent;
        delay(1000); // Don't let noise ruin it
    }
}

void ascent_f() {
    if (verticalSpeed < descentVSpeedThreshold) {
        state = descent;
    }
}

void descent_f() {
    if (currentAlt < targetAltitude) {
        // Start cutting sequence
        //nichrome.start(currentTime, cutTime);
        digitalWrite(11, HIGH);
        delay(6000);
        digitalWrite(11, LOW);
        ledBlinkRate = 250;
        state = deploy;
    }
}

void deploy_f() {
    //nichrome.hold(currentTime);

    // Don't burn the field
    if (currentAlt < forceDeployThreshold && currentAlt > cutoffAlt) {
        digitalWrite(11, HIGH);
        delay(6000);
        digitalWrite(11, LOW);
        nichrome.end();
        ledBlinkRate = 1000;
    }

    // Play buzzer
    buzzer.play();
}

// Round off numbers before transmitting. Cuz data.
float rnd(float f, int r) {
    float roundVal = pow(10, r);
    // Round float to 2 decimal points
    return (float)((int)(f * roundVal)) / roundVal;
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
