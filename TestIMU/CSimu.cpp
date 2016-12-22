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
bool CSimu::debugMode;
bool CSimu::useGroundAltitude;

double CSimu::groundAltitude;
double CSimu::pressureVal;


CSimu::CSimu() {
    // Options
    debugMode = false;
    useGroundAltitude = true;
    accelInMetersPerSecond = true;
    altInMeters = true;
    pressInHectopascals = true;

    // inits
    groundAltitude = 0;
    pressureVal = 0;
}

Adafruit_BMP085_Unified CSimu::barometer;
Adafruit_L3GD20 CSimu::gyroscope;
Adafruit_LSM303 CSimu::accelMag;
//sensors_event_t CSimu::event; // meh

void CSimu::debug(String s) {
    if (debugMode) {
        Serial.println(s);
    }
}

void CSimu::updateSensors() { // Call before accessing data
    gyroscope.read(); // update gyroscope.data variable.
    accelMag.read(); // Does both the accelerometer and magnetometer
    accel.x = accelMag.accelData.x;
    accel.y = accelMag.accelData.y;
    accel.z = accelMag.accelData.z;

    gyro.x = gyroscope.data.x;
    gyro.y = gyroscope.data.y;
    gyro.z = gyroscope.data.z;

    mag.x = accelMag.magData.x;
    mag.y = accelMag.magData.y;
    mag.z = accelMag.magData.z;

    pressure = getPressure();
    temperature = getTemperature();
    altitude = getAltitude();
}


double CSimu::getTemperature() {
    float t;
    barometer.getTemperature(&t);
    return t;
}

double CSimu::getPressure() {
    float p;
    barometer.getPressure(&p);
    pressureVal = p;

    if (pressInHectopascals) {
        return p / 100.00;
    } else {
        return p;
    }

}

double CSimu::getAltitude() {
    // Feed pressure at sea level and current pressure (in hPa!)
    // Pressure at sea level (1 atm) = 1013 hPa
    // Can return true alt or 'radar' alt
    double currentAltitude = barometer.pressureToAltitude(1013.0, pressureVal); // Supposedly metres, but looks like feet.

    //Serial.println("GetAltitude()");
    if (!altInMeters) {
        //Serial.println("1");
        if (useGroundAltitude) {
            //Serial.println("1.1)");
            return currentAltitude - groundAltitude;
        } else {
            //Serial.println("1.2");
            return currentAltitude;
        }
    } else {
        // Use press and temp to get alt in metres!
        // formula: a * (b / c)
            // Where a = ( (p0/p)^(1/5.257) ) - 1
        Serial.println("2");
        double a;
        if (pressInHectopascals) {
            //Serial.println("2.1.1");
            a = pow((1013.25 / pressure), (1 / 5.257)) - 1;
        } else {
            //Serial.println("2.1.2");
            a = pow((1013.25 / (pressure / 100.0)), (1 / 5.257)) - 1;
        }
        double b = temperature + 273.15;
        double c = 0.0065;

        if (useGroundAltitude) {
            //Serial.println("2.2.1");
            return a * b / c - groundAltitude;
        } else {
            //Serial.println("2.2.2");
            return a * b / c;
        }
    }

}



void CSimu::config() {
    debug("CSimu.config():");

    // ***** Barometer stuff
        barometer.begin();

    // ***** gyroscope stuff
    //gyroscope.begin(GYRO_RANGE_250DPS, 0x00); // Specify gyroscope's I2C address!
        gyroscope.begin();

    // ***** Accel and mag stuff
    // begin, set mag gain (?)
        accelMag.begin();
        accelMag.setMagGain(LSM303_MAGGAIN_1_3); // Maybe? Find stuff on C++ enums

    // *** Vector initializations
        gyro = Vector();
        accel = Vector();
        mag = Vector();
}

void CSimu::setGroundAltitude(float init) {
    groundAltitude = init;
}

Vector::Vector() {
    x = 0;
    y = 0;
    z = 0;
}

void CSimu::autoSetGroundAltitude() {
    // Sample altitude 20 times, take average (and debug if necessary!)
    // Don't use if you won't have power on the launchpad!
        // use setGroundAltitude() instead
    updateSensors();
    double sum = 0;
    Serial.println("Sampling ground altitude");
    for (int i = 1; i <= 10; i++) {
        double thing = altitude;
        sum = sum + thing;
        if (debugMode) {
            Serial.print("\tSample ");
            Serial.print(i);
            Serial.print(" ");
            Serial.println(thing);
        }
        delay(20);
        updateSensors();
    }
    groundAltitude = sum / 10;
    if (debugMode) {
        Serial.print("\tGround alt was found to be ");
        Serial.println(groundAltitude);
    }

}

double CSimu::gAccelMag() {
    // Assumes that 1g = 1024u
    // sqrt(x^2 + y^2 + z^2)
    // where x is converted to g's
    double a = accel.x / 1024;
    double b = accel.y / 1024;
    double c = accel.z / 1024;

    return sqrt(a * a + b * b + c * c);
}
