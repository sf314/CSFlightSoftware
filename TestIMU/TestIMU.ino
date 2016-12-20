// Can't be called imu.ino for reasons!
// Also can't be named the same thing as a file in the folder, so TestIMU.ino it is!

// Note: SCL is A5 and SDA is A4s

// Calibration:
// imu.temperature GOOD
// imu.pressure() = 97724 pascals = 0.96 atm = 313m/1,140ft elev. (Tempe = 1,125ft) GOOD
// imu.altitude() = 313m ??? 24@3st     54@6st     3@0st ITS RETURNING FEET!
    // altitude preset starting value: 105760 (to compensate)


#include <Arduino.h>
#include <Wire.h>
#include "CSimu.h"

CSimu imu = CSimu();

void setup() {
    Serial.begin(9600);
    delay(100);

    imu.debugMode = true;
    imu.useGroundAltitude = true;
    imu.setInitialAltitude(105760);
    imu.config();
    for (int i; i < 5; i++) {
        Serial.print(i); Serial.print(" ");
        delay(1000);
    }
}

int gx, gy, gz;
int ax, ay, az;
int mx, my, mz;
float press, temp, alt;

void loop() {
    imu.updateSensors();
    getData();

    // Choose which data to show 0, 1, 2, 3
    int sensor = 3;
    switch (sensor) {
        case 0:
            showGyroData();
            break;
        case 1:
            showAccelData();
            break;
        case 2:
            showMagData();
            break;
        case 3:
            showOther();
            break;
        default:
            Serial.println("invalid sensor value");
    }

    delay(200);
}

void getData() {
    gx = imu.gyroX();
    gy = imu.gyroY();
    gz = imu.gyroZ();
    ax = imu.accelX();
    ay = imu.accelY();
    az = imu.accelZ();
    mx = imu.magX();
    my = imu.magY();
    mz = imu.magZ();
    press = imu.pressure();
    temp = imu.temperature();
    alt = imu.altitude();
}

void showGyroData() {
    Serial.println("Gyro = " + String(gx) + " | " + String(gy) + " | " + String(gz));
}
void showAccelData() {
    Serial.println("Accel = " + String(ax) + " | " + String(ay) + " | " + String(az));
}
void showMagData() {
    Serial.println("Mag = " + String(mx) + " | " + String(my) + " | " + String(mz));
}
void showOther() {
    Serial.println("P = " + String(press) + "\nT = " + String(temp) + "\nA = " + String(alt));
}
