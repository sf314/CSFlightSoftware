// Inertial Measurement Unit class definition
// 10DOF
// CSimu.h
// may need float rather than int (data.x is int for some reason)


// *** FINISHED IMPLEMENTING: (and needs testing)
/*
gyro
pressure
temperature
altitude

*/

#include "Arduino.h"

// From Adafruit folder (messy, yes!)
#include "Adafruit_10DOF.h"
#include "Adafruit_BMP085_U.h"
#include "Adafruit_L3GD20_U.h"
#include "Adafruit_LSM303_U.h"
#include "Adafruit_Sensor.h"


class CSimu {
public:

    void config();
    static bool debugMode; // default false
    static bool useGroundAltitude; // if ground level is 0m altitude, default false

    void updateSensors();

    int accelX();
    int accelY();
    int accelZ();

    int gyroX();
    int gyroY();
    int gyroZ();

    int magX();
    int magY();
    int magZ();

    float temperature(); // in C
    float pressure(); // in kPa
    float altitude(); // in m (assumes sea level pressure is 1013 hPa)


private:
    //sensors_event_t event; // meh
    static Adafruit_BMP085_Unified barometer;
    static Adafruit_L3GD20 gyro; // not the unified one
    static Adafruit_LSM303 accelMag; // the same sensor, and not the unified class

    static float groundAltitude;
    static float pressureVal; // for pressure() and altitude()

    void calibrate();
    void debug(String s);
};
