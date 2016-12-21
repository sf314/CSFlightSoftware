//
// CSimu.cpp
//
//
// Created by Stephen Flores on 10/17/2016
//
//

#include <Arduino.h>
#include <Wire.h>
#include "CSimu.h"

/*
Required:
Wire.begin(); // Very important!
CSimu imu;
imu.config(); // Handles all configs

*/

bool CSimu::debugMode = false;
bool CSimu::useGroundAltitude = false;
float CSimu::groundAltitude = 0; // Centimetres?
float CSimu::pressureVal = 0;

Adafruit_BMP085_Unified CSimu::barometer;
Adafruit_L3GD20 CSimu::gyro;
Adafruit_LSM303 CSimu::accelMag;
//sensors_event_t CSimu::event; // meh

void CSimu::debug(String s) {
    Serial.println(s);
}

void CSimu::updateSensors() { // Call before accessing data
    gyro.read(); // update gyro.data variable.
    accelMag.read(); // Does both the accelerometer and magnetometer
}

int CSimu::gyroX() { return gyro.data.x; }
int CSimu::gyroY() { return gyro.data.y; }
int CSimu::gyroZ() { return gyro.data.z; }

int CSimu::accelX() {return accelMag.accelData.x;}
int CSimu::accelY() {return accelMag.accelData.y;}
int CSimu::accelZ() {return accelMag.accelData.z;}

int CSimu::magX() {return accelMag.magData.x;}
int CSimu::magY() {return accelMag.magData.y;}
int CSimu::magZ() {return accelMag.magData.z;}


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
    float currentAltitude = barometer.pressureToAltitude(1013.0, pressureVal); // Supposedly metres, but looks like feet.

    if (useGroundAltitude) {
        return currentAltitude - groundAltitude; // return diff
        Serial.print("From ground");
    }
    return currentAltitude;
}



void CSimu::config() {
    debug("CSimu.config():");

    // ***** Barometer stuff
        barometer.begin();

    // Calculate ground altitude by averaging 20 samples
        debug("\tSampling ground altitude");
        float sum = 0;
        for (int i = 0; i < 20; i++) {
            sum = sum + altitude();
        }
        groundAltitude = sum / 20;

    // ***** Gyro stuff
    //gyro.begin(GYRO_RANGE_250DPS, 0x00); // Specify gyro's I2C address!
        gyro.begin();

    // ***** Accel and mag stuff
    // begin, set mag gain (?)
        accelMag.begin();
        accelMag.setMagGain(LSM303_MAGGAIN_1_3); // Maybe? Find stuff on C++ enums
}


void CSimu::setInitialAltitude(float init) {
    groundAltitude = init;
}
