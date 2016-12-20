// Inertial Measurement Unit class definition
// 10DOF
// CSimu.h
// may need float rather than int

#include "Arduino.h"

// From Adafruit folder (messy, yes!)
#include "Adafruit/Adafruit_10DOF.h"
#include "Adafruit/Adafruit_BMP085_U.h"
#include "Adafruit/Adafruit_L3GD20_U.h"
#include "Adafruit/Adafruit_LSM303_U.h"
#include "Adafruit/Adafruit_Sensor.h"


class CSimu {
public:

    void config();
    static bool debugMode;
    static bool useGroundAltitude; // if ground level is 0m altitude

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
    Adafruit_BMP085_Unified barometer;
    Adafruit_L3GD20 gyro; // not the unified one

    float groundAltitude;
    float pressureVal; // for pressure() and altitude()

    void calibrate();
    void debug(String s);
}
