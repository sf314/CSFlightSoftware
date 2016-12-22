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


class Vector {
public:
    double x;
    double y;
    double z;
    Vector();
};


class CSimu {
public:
    // Initializer
    CSimu();

    void config();

    // Options
    static bool debugMode; // default false
    static bool useGroundAltitude; // if ground level is 0m altitude, default false // 105760 for tempe warehouses
    bool accelInMetersPerSecond; // Adjustment made in getAcceleration()
    bool altInMeters; // Adjustment made in getAltitude();
    bool pressInHectopascals; // Made in getPressure();


    void updateSensors();

    void setGroundAltitude(float init); // preset for area or for sea level, overrides averaging that config() does
    void autoSetGroundAltitude();

    // ****** NEW STUFF:
    Vector accel;
    Vector gyro;
    Vector mag;

    double pressure;
    double temperature;
    double altitude;

    double gAccelMag(); // magnitude of acceleration vector in g's

private:
    //sensors_event_t event; // meh
    static Adafruit_BMP085_Unified barometer;
    static Adafruit_L3GD20 gyroscope; // not the unified one
    static Adafruit_LSM303 accelMag; // the same sensor, and not the unified class

    double getTemperature(); // in C
    double getPressure(); // in kPa
    double getAltitude(); // in m (assumes sea level pressure is 1013 hPa)

    static double groundAltitude;
    static double pressureVal; // for pressure() and altitude()

    void calibrate(); // unimplemented and unused!
    void debug(String s);
};
