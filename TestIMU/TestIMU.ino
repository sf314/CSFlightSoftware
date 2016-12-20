// Can't be called imu.ino for reasons!
// Also can't be named the same thing as a file in the folder, so TestIMU.ino it is!


#include <Arduino.h>
#include <Wire.h>
#include "CSimu.h"

CSimu imu = CSimu();

void setup() {
    Serial.begin(9600);
    delay(100);

    imu.config();
    imu.debugMode = true;
}

int gx, gy, gz;
int ax, ay, az;
int mx, my, mz;
float press, temp, alt;

void loop() {
    imu.updateSensors();
    getData();

    // Choose which data to show 0, 1, 2, 3
    int sensor = 0;
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

    delay(1000);
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
