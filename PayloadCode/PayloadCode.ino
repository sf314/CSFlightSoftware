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
// <ALTITUDE>, <PRESSURE>,<VELOCITY>, <TEMP>,<VOLTAGE>,
// <HEADING>,<SOFTWARE STATE>

// Calibration:
// imu.temperature GOOD
// imu.pressure() = 97724 pascals = 0.96 atm = 313m/1,140ft elev. (Tempe = 1,125ft) GOOD
// imu.altitude() = 313m ??? 24@3st     54@6st     3@0st ITS RETURNING FEET!
    // altitude preset starting value: 105760 (to compensate)

#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>
#include "CSimu.h"
#include "CSCoreData.h"
// Voltage divider on pin 14

// ********** Mission Constants ***********************************************
#define descendingSpeedThreshold -1
#define landedAltThreshold 10


// ********** Objects *********************************************************
SoftwareSerial xbee(1, 0); String CSComms_dataString = "";
CSimu imu = CSimu();
CSCoreData coreData;

int buzzerPin = 12;
bool playBuzzer = true; bool buzzerIsOn = false;

// ********** Telemetry *******************************************************
double missionTime = 0;
int packetCount = 0;
double currentAlt = 0;
    double previousAlt = 0;
    double verticalSpeed = 0;
double pressure = 0;
double velocity = 0;
double temperature = 0;
double voltage = 0;
double heading = 0;

// ********** Common Grounds **************************************************
double tempeAlt = 372.0;

// ********** State info ******************************************************
int state = 0;
    const int boot = 0;
    const int descent = 1;
    const int landed = 2;

// ********** Time vars *******************************************************
long currentTime = 0;
long previousTime = 0;
long restoredTime = 0;
int delayTime = 1000;

void setup() {
    Serial.begin(9600);
    xbee.begin(9600);
    Wire.begin();
    Serial.println("Hey");

    // IMU stuff
    imu.config();
    delay(50);
    imu.setGroundAltitude(tempeAlt);

    imu.updateSensors();
    currentAlt = imu.altitude;
    previousAlt = currentAlt;

    // Temp sensor
    pinMode(23, INPUT);

}

void loop() {

    // *** Update time
    currentTime = getTime();

    // *** Listen for commands from XBee
    if (xbee.available()) {
        char c = xbee.read();
        Serial.print("Received "); Serial.println(c);
        CSComms_parse(c);
    }

    // *** Listen for commands from XBee
    if (Serial.available()) {
        char c = Serial.read();
        Serial.print("Received "); Serial.println(c);
        CSComms_parse(c);
    }

    // *** Main code
    if (currentTime - previousTime >= delayTime) {
        // Update sensor data
        imu.updateSensors();

        // Collect official telemetry data
        missionTime = (double)currentTime / 1000.0;
        currentAlt = imu.altitude;
        pressure = imu.pressure;
        velocity = -2; // ? Hardcoded for now? Replace with smart thing
        temperature = readTemp(23);
        voltage = readVolt(14);

        // Do calculations
        float deltaT = (float)(currentTime - previousTime) / 1000.0;
        verticalSpeed = (currentAlt - previousAlt) / deltaT;
        velocity = abs(verticalSpeed); // Force velocity to be verticalSpeed

        // Add to data string (only print in descent)
        CSComms_dataString = "";
        CSComms_add("2848,GLIDER,");
        CSComms_add(missionTime);
        CSComms_add(packetCount);
        CSComms_add(currentAlt);
        CSComms_add(pressure);
        CSComms_add(velocity);
        CSComms_add(temperature);
        CSComms_add(voltage);
        CSComms_add(heading);
        CSComms_add(state);

        Serial.println(CSComms_dataString);

        // Switch on state
        switch (state) {
            case descent:
                descent_f();
                break;
            case landed:
                landed_f();
                break;
            case boot:
                boot_f();
                break;
            default:
                Serial.println("Invalid state");
                state = boot;
                break;
        }

        // Persist time, packet count, and groundAltitude (or hardcode?)

        // Set previous stuff
        previousAlt = currentAlt;
        previousTime = currentTime;

    }

}










// ********** Do a smart thing with time **************************************
long getTime() {
    // Maintain restored time here
    long temp = millis() + restoredTime;
    return temp;
}



// ********** State funcs *****************************************************
void boot_f() {
    // Restore values from previous
    restoredTime = coreData.restoreTime();
    packetCount = coreData.restorePacketCount();
    state = descent;
}

void descent_f() {
    // Send telemetry
    CSComms_transmit();

    // State change condition
    if (verticalSpeed >= descendingSpeedThreshold) {
        state = landed;
    }
}

void landed_f() {
    Serial.println("Landed");

    // Activate buzzer if desired
    if (buzzerIsOn) {
        digitalWrite(buzzerPin, LOW);
        buzzerIsOn = false;
    } else {
        if (playBuzzer) {
            digitalWrite(buzzerPin, HIGH);
        }
        buzzerIsOn = true;
    }

    // State change condition (maybe landed state was error?)
    if (verticalSpeed < descendingSpeedThreshold) {
        state = descent;
    }

    // NOTE: For testing, force transmission during landed state
    CSComms_transmit();
}

// ********** CSComms *********************************************************
void CSComms_add(int l) {
    CSComms_dataString += String(l) + ",";
}

void CSComms_add(double f) {
    CSComms_dataString += String(f) + ",";
}

void CSComms_add(String s) {
    CSComms_dataString += s;
}

void CSComms_log(String s) {
    xbee.println(s);
}

void CSComms_transmit() {
    xbee.println(CSComms_dataString);
    CSComms_dataString = "";
    packetCount++;
}

bool CSComms_available() {
    return xbee.available();
}

char CSComms_read() {
    return xbee.read();
}

void CSComms_begin(int baud) {
    xbee.begin(baud);
}

void CSComms_parse(char c) {
    switch (c) {
        case 'b':
            if (playBuzzer) { // Toggle buzzer (cannot be toggled if landed)
                playBuzzer = false;
            } else {
                playBuzzer = true;
            }
            break;
        case '8':
            state = descent; // Force state 1
            break;
        case '9':
            state = landed;
            break;
        case 'w':       // Wipe EEPROM to all zeroes
            coreData.wipe();
        default:
            xbee.println("Invalid command");
    }
}






// ********** Temperature reading *********************************************
double readTemp(int tempPin) {
    // IF 5V, use 5000.0
    // If 3.3V, use 3300.0

    double pinMillivolts = analogRead(tempPin) * (5000.0 / 1024.0);
    double celsius = (pinMillivolts - 500.0) / 10.0; // in celsius

    // perform other calculations, or switch on a thing to use Fahrenheit
    //double fahrenheit =  celsius * 1.8 + 32;
    return celsius;
}


// Voltage reading

double readVolt(int pin) {
    double pinMillivolts = analogRead(pin) * (3300.0 / 1024.0);
    double volts = pinMillivolts / 1000.0;
    //return volts * 2;
    // Or do some rubbish
    return 6.0;
}
