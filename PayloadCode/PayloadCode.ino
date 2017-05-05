// Stephen Flores
// Glider software
// PayloadCode.ino

// NOTE: Currently, the Payload only sends some XBee data and handles the
// buzzer. The buzzer can be toggled using the 'b' command. However, the
// buzzer cannot be toggled when landed. The landed state forces the buzzer on.
// Currently, there are only two states recognized: descent and landed. More
// will be added.
// Telemetry only saves accel and alt values for testing.

// ********** Inclusions ******************************************************
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <math.h>
#include <SoftwareSerial.h>
#include "CSBuzzer.h"
#include "CSimu.h"
#include "CSLog.h"
#define pi 3.14159265358

// ********** Objects *********************************************************
SoftwareSerial xbee(0, 1); String CSComms_dataString = "";
CSBuzzer buzzer; bool playBuzzer = false;
CSimu imu;

// ********** Time variables **************************************************
int refreshRate = 1000;
long currentTime = 0;
long previousTime = 0;

// ********** State information ***********************************************
int state = 0;
    int boot = 0;
    int descent = 0;
    int landed = 1;

// ********** Telemetry structure *********************************************
float currentAlt = 0;
float previousAlt = 0;
float verticalSpeed = 0;

// ********** Setup ****************************************************#######
void setup() {
    Serial.begin(9600);

    // Communications
    CSComms_begin(9600);

    // Buzzer
    buzzer = CSBuzzer(6);
    buzzer.setFrequency(800);

    // IMU
    imu.config();
    imu.autoSetGroundAltitude();
}

// ********** Loop *****************************************************#######
void loop() {
    // Update time
    currentTime = millis();

    // Check for commands
    if (CSComms_available()) {
        char c = CSComms_read();
        Serial.println("Received: " + String(c));

        CSComms_parse(c);
    }

    // Run main code
    if (currentTime - previousTime > refreshRate) {

        // *** Collect and save telemetry
        imu.updateSensors();
        currentAlt = imu.altitude;
        verticalSpeed = (currentAlt - previousAlt) / (float)refreshRate;
        previousAlt = currentAlt;
        CSComms_add((long)state);
        CSComms_add((float)imu.accel.x);
        CSComms_add((float)imu.accel.y);
        CSComms_add((float)imu.accel.z);
        CSComms_add(currentAlt);

        // Print CSComms_dataString to serial for debugging
        Serial.println(CSComms_dataString);

        // Save to SD for later!

        // *** Switch on state
        switch (state) {
            case 0:
                boot_f();
                break;
            case 1:
                descent_f();
                break;
            case 2:
                landed_f();
                break;
            default:
                Serial.println("Invalid state");
                CSComms_log("Invalid state");
                state = descent;
        }

        // Play buzzer if necessary (it beeps @ refreshRate)
        if (playBuzzer) {
            buzzer.play();
        }

        previousTime = currentTime;
    }
}


// ********** State funcs ******************************************************
void boot_f() {
    // Recover state and time from reboot!
}

void descent_f() {
    // Oi
    // Send telemetry only in this state
    CSComms_transmit();

    // State change conditions: below alt threshold
    // Other state change would be verticalspeed, but testing would be weird
}

void landed_f() {
    // Force the buzzer
    playBuzzer = true;

    // State change conditions: descending, above alt threshold
}



// ********** XBee funcs ******************************************************
void CSComms_add(long l) {
    CSComms_dataString += String(l) + ",";
}

void CSComms_add(float f) {
    CSComms_dataString += String(f) + ",";
}

void CSComms_add(String s) {
    CSComms_dataString += s;
}

void CSComms_log(String s) {
    xbee.println(s);
}

void CSComms_transmit() {
    xbee.println(CSComms_dataString);
    CSComms_dataString = "";
    //packetCount++;
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
        case 'b':
            if (playBuzzer) { // Toggle buzzer (cannot be toggled if landed)
                playBuzzer = false;
            } else {
                playBuzzer = true;
            }
            break;
        case '0':
            state = descent;
            break;
        case '1':
            state = landed;
            break;
        default:
            xbee.println("Invalid command");
    }
}







// ********** Calculate Heading based on Hx, Hy (adafruit) ********************
// May need to change axes in order to calc it right
// Assumes that X direction is prograde
    // If Y is prograde, then feed Y as X, and -X as Y
    // Should be fine based on schematics
    // If backwards (-X is prograde), feed -X as X and -Y as Y
int headingFromIMU(float x, float y) {
    float returnValue = 0.0;
    if (y > 0) {
        returnValue = (int)(90 - (atan(x / y)) * ( 180 / pi));
    } else if (y < 0) {
        returnValue = (int)(270 - (atan(x / y)) * (180 / pi));
    } else if (y == 0 && x < 0) {
        returnValue = 180;
    } else if (y == 0 && x > 0) {
        returnValue = 0;
    }

    return (int)(round(returnValue));
}
