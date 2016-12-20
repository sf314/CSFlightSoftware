//
// CSimu.cpp
//
//
// Created by Stephen Flores on 10/17/2016
//
//

#include <Arduino.h>
#include <Wire.h>
#include <CSimu.h>

/*
Required:
Wire.begin(); // Very important!
CSimu imu;
imu.config(); // Handles all configs

*/

bool CSimu::debugMode = false;
bool CSimu::useGroundAltitude = false;
float CSimu::groundAltitude = 0;

Adafruit_BMP085_Unified CSimu::barometer;
sensors_event_t CSimu::event;

void CSimu::debug(String s) {
    Serial.println(s);
}

void CSimu::config() {
    debug("CSimu.config():");

    // ***** Barometer stuff
    barometer.begin();

    // Calculate ground altitude by averaging 20 samples
    debug("\tSampling ground altitude");
    float sum = 0;
    for (int i = 0; i < 20; i++) {
        sum = sum + altutude();
    }
    groundAltitude = sum / 20;

    // ***** Gyro stuff
    gyro.begin(gyro.L3DS20_RANGE_250DPS, 0x00); // Specify gyro's I2C address!

}

void CSimu::updateSensors() {
    gyro.read(); // update gyro.data variable.
}

int CSimu::gyroX() { return gyro.data.x; }
int CSimu::gyroY() { return gyro.data.y; }
int CSimu::gyroZ() { return gyro.data.z; }


float CSimu::temperature() {
    float t;
    barometer.getTemperature(&t);
    return t;
}

float CSimu::pressure() {
    float p;
    barometer.getPressure(&p);
    pressureVal = p;
    return p;
}

float CSimu::altitude() {
    // Feed pressure at sea level and current pressure (in hPa!)
    // Pressure at sea level (1 atm) = 1013 hPa
    // Can return true alt or 'radar' alt
    float currentAltitude = barometer.pressureToAltitude(1013.0, pressureVal;

    if (useGroundAltitude) {
        return currentAltitude - groundAltitude; // return diff
    }
    return currentAltitude;
}
