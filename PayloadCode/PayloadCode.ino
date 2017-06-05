// Stephen Flores
// Glider software
// PayloadCode.ino

// NOTE: Currently, the Payload only sends some XBee data and handles the
// buzzer. The buzzer can be toggled using the 'b' command. However, the
// buzzer cannot be toggled when landed. The landed state forces the buzzer on.
// Currently, there are only two states recognized: descent and landed. More
// will be added.
// Telemetry format:
// <TEAM ID>,GLIDER,<MISSION TIME>,<PACKET COUNT>,
// <ALTITUDE>, <PRESSURE>,<SPEED>, <TEMP>,<VOLTAGE>,
// <HEADING>,<SOFTWARE STATE>

// TODO:
//ADD TEMP
//

// ********** Inclusions ******************************************************
#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include <SPI.h>
#include <SD.h>
#include <math.h>
#include <SoftwareSerial.h>
#include "CSBuzzer.h"
#include "CSimu.h"
#include "CSLog.h"
#include "CSPitot.h"
#include "CSCoreData.h"
#include "CSVolt.h"
#define pi 3.14159265358

// ********** Mission constants ***********************************************
// Below 20 is considered landed
#define LANDED_THRESHOLD 20

// ********** Objects *********************************************************
SoftwareSerial xbee(0, 1); String CSComms_dataString = "";
CSBuzzer buzzer; bool playBuzzer = false;
CSimu imu;
CSPitot pitot;
CSCoreData coreData;
CSVolt volt;
bool ledOn = false;

// ********** Time variables **************************************************
int refreshRate = 1000;
long currentTime = 0;
long previousTime = 0;
long restoredTime = 0;

// ********** State information ***********************************************
int state = 0;
    const int boot = 0;
    const int descent = 1;
    const int landed = 2;

// ********** Telemetry structure *********************************************
int packetCount = 0;
float currentAlt = 0;
float pressure = 0;
float velocity = 0;
float temp = 0;
float voltage = 0;
float heading = 0;

float verticalSpeed = 0;
float previousAlt = 0;

// ********** Setup ****************************************************#######
void setup() {
    pinMode(13, OUTPUT);
    digitalWrite(13, HIGH);
    Serial.begin(9600);
    Serial.println("Entering setup");
    Wire.begin();

    // Communications
    CSComms_begin(9600);

    // Buzzer
    buzzer = CSBuzzer(22);
        // NOTE:
        // Buzzer 1 is positive 22 // Make them different to be on (pull 22 high)
        // Buzzer 2 is negative 21
    buzzer.setFrequency(800);
    // Temp is 23
    //

    // IMU
    imu.config();
    imu.autoSetGroundAltitude();

    // Pitot
    pitot.setAddress(0x46);

    // Voltage divider
    volt = CSVolt(14); // What pin?
    Serial.println("Ending setup");
}

// ********** Loop *****************************************************#######
void loop() {
    // Update time
    currentTime = millis() + restoredTime;

    // Check for commands
    if (CSComms_available()) {
        char c = CSComms_read();
        Serial.println("Received: " + String(c));

        CSComms_parse(c);
    }

    // Run main code
    if (currentTime - previousTime > refreshRate) {
        Serial.println("Starting man runloop");

        // *** Collect and save telemetry
        updateTelemetry();
        float tempTime = (float)currentTime / 1000.0;
            // 1: Comms
            CSComms_add("2848,GLIDER,");
            CSComms_add(tempTime);
            CSComms_add(packetCount);
            CSComms_add(currentAlt);
            CSComms_add(pressure);
            CSComms_add(velocity);
            CSComms_add(temp);
            CSComms_add(voltage);
            CSComms_add(heading);
            CSComms_add(state);

            // 2: SD (add more data!)
            SD_add("2848,GLIDER");
            SD_add(tempTime);
            SD_add(packetCount);
            SD_add(currentAlt);
            SD_add(pressure);
            SD_add(velocity);
            SD_add(temp);
            SD_add(voltage);
            SD_add(heading);
            SD_add(state);


        // Print CSComms_dataString to serial for debugging
        Serial.println(CSComms_dataString);

        // *** Switch on state
        switch (state) {
            case boot:
                boot_f();
                break;
            case descent:
                descent_f();
                break;
            case landed:
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

        if (ledOn) {
            digitalWrite(13, LOW);
            ledOn = false;
        } else {
            digitalWrite(13, HIGH);
            ledOn = true;
        }

        previousTime = currentTime;
        Serial.println("Ending main runloop");
    }
}


// ********** State funcs ******************************************************
void boot_f() {
    // Restore time and packets if possible
    restoredTime = coreData.restoreTime();
    packetCount = coreData.restorePacketCount();
}

void descent_f() {
    // Oi
    // Send telemetry only in this state
    CSComms_transmit();

    // State change conditions: below alt threshold
    // Other state change would be verticalspeed, but testing would be weird
    if (currentAlt < LANDED_THRESHOLD) {
        state = landed;
    }
}

void landed_f() {
    // Force the buzzer
    playBuzzer = true;

    // Don't transmit telemetry

    // State change conditions: descending, above alt threshold
    if (currentAlt > LANDED_THRESHOLD) {
        state = descent;
    }
}



// ********** XBee funcs ******************************************************
void CSComms_add(int l) {
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
        case 'b':
            if (playBuzzer) { // Toggle buzzer (cannot be toggled if landed)
                playBuzzer = false;
            } else {
                playBuzzer = true;
            }
            break;
        case '8':
            state = descent; // Force state 0 or 1
            break;
        case '9':
            state = landed;
            break;
        case 'w':       // Wipe EEPROM to all zeroes
            coreData.wipe();
        default:
            xbee.println("Invalid command");
    }
}

void updateTelemetry() {
    imu.updateSensors();
    currentAlt = imu.altitude;
        verticalSpeed = (1000) * (currentAlt - previousAlt) / (float)refreshRate;
        previousAlt = currentAlt;
    pressure = imu.pressure;
    velocity = pitot.getVelocity();
    temp = imu.temperature; // or from TMP36?
    voltage = volt.read();

    heading = headingFromIMU(imu.mag.x, imu.mag.y);

    // Update EEPROM
    coreData.storeTime(currentTime);
    coreData.storePacketCount(packetCount);
}





// ********** Calculate Heading based on Hx, Hy (adafruit) ********************
// Ey! Do something smarter!
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
