// Cansat 2017 Container software
// Must access CSAlt, CSComms, and CSLog
// Requirements:
// Transmit telemetry at 1Hz (rq. fields in req. format)
// Deploy payload (via payload release function) at 400m radar alt
// Activate beacon and stop telemetry 2s after deploying payload

// NOTE:
// SD filename, headers, and pin must be manually set at the top of CSLog.h

// TODO:
// Implement state switch logic

// ********** Required Libraries **********************************************
#include "Wire.h"
#include "SPI.h"
#include "SD.h"
#include "IntersemaBaro.h"
#include "CSAlt.h"
#include "CSLog.h"
//#include "CSComms.h"

// ********** Objects *********************************************************
CSAlt baro;
CSLog sdLog;
//CSComms comms;

// ********** Control variables ***********************************************
int state           = 0;
    int boot        = 0;
    int launchpad   = 1;
    int ascent      = 2;
    int descent     = 3;
    int deploy      = 4;

// ********** Time variables **************************************************
long currentTime    = 0;
long previousTime   = 0;
long  startTime     = 0;
long restoredTime   = 0;
long prevTelemTime  = 0;

// ********** Telemetry structure *********************************************

// ********** Setup Function *********************************************#####
void setup() {
    Serial.begin(9600);
    delay(1000);

    baro.init();
    baro.setGroundHeight(baro.alt()); // set startup ground height as zero. 

    // Restore state and packet count if from reset (and log to local telem)
    startTime = setStartTime();
}

// ********** Loop Function **********************************************#####
void loop() {
    currentTime = millis() - startTime;

    // *** Non-time triggered functions. Run every time.

    // *** Time-triggered functions. Must happen on every iteration
    if (currentTime - prevTelemTime > 1000) {
        // * Get data
        float alt = baro.altRadar();


        // * Save data
        sdLog.add(currentTime);
        sdLog.add(alt);
        sdLog.saveTelemetry();
        //comms.transmitTelemetry();
    }
}






// ********** Miscellaneous functions *****************************************
long setStartTime() {
    // TODO: Recover time t from file if it exists
    return millis(); // do something smarter(?) eventually
}
