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

// Calibration:
// imu.temperature GOOD
// imu.pressure() = 97724 pascals = 0.96 atm = 313m/1,140ft elev. (Tempe = 1,125ft) GOOD
// imu.altitude() = 313m ??? 24@3st     54@6st     3@0st ITS RETURNING FEET!
    // altitude preset starting value: 105760 (to compensate)

#include <Arduino.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include "CSimu.h"

// ********** Mission Constants ***********************************************
#define isDescendingSpeedThreshold -1
#define isLandedAltThreshold 10


// ********** Objects *********************************************************
CSimu imu = CSimu();
SoftwareSerial xbee(0, 1);

// ********** Telemetry *******************************************************
float missionTime = 0;
int packetCount = 0;
float currentAlt = 0;
    float prevousAlt = 0;
    float verticalSpeed = 0;
float pressure = 0;
float velocity = 0;
float temperature = 0;
float voltage = 0;
float heading = 0;

// ********** Common Grounds **************************************************
float tempeAlt = 372.0;

// ********** State info ******************************************************
int state = 0;
    const int boot = 0;
    const int descent = 1;
    const int landed = 2;

// ********** Time vars *******************************************************
long currentTime = 0;
long previousTime = 0;
long restoredTime = 0;
long bootTime = 0;
int delayTime = 1000;

void setup() {
    Serial.begin(9600);
    Wire.begin();

    imu.config();
    delay(50);
    imu.setGroundAltitude(tempeAlt);

    imu.updateSensors();
    currentAlt = imu.altitude;
    prevousAlt = currentAlt;
}

void loop() {

    // *** Update time
    currentTime = getTime();

    // *** Listen for commands
    if (xbee.available()) {
        char c = xbee.read();
    }

    // *** Main code
    if (currentTime - previousTime >= delayTime) {
        // Update sensor data
        imu.updateSensors();

        // Collect official telemetry data
        missionTime = ()(double)currentTime / 1000.0);
        altitude = imu.altitude;
        pressure = imu.pressure;
        velocity = -2; // ? Hardcoded for now? Replace with smart thing
        temperature = imu.temperature;
        voltage = 0; // Hardcoded for now. Do other stuff

        // Run some calculations


        // Switch on state
        switch (state) {
            case descent:
                break;
            case landed:
                break;
            case boot:
                break;
            default:
                Serial.println("Invalid state");
                state = boot;
                break;
        }

        // Send telemetry

        // Set previous stuff
        previousTime = currentTime;

    }

}










// ********** Do a smart thing with time **************************************
long getTime() {
    // Maintain restored time here
    long temp = millis();
    return temp;
}



// ********** State funcs *****************************************************
void descent_f() {

    // State change
}

void landed_f() {

    // State change
}

// ********** CSComms *********************************************************
