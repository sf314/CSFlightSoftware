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
#define pi 3.1415926535897932384626433832795


// ********** Objects *********************************************************
SoftwareSerial xbee(1, 0); String CSComms_dataString = "";
CSimu imu = CSimu();
CSCoreData coreData;

int buzzerPin = 12;
bool playBuzzer = true; bool buzzerIsOn = false;
bool ledOn = false;

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

// ********** For Ground Persistence ******************************************
double tempeAlt = 372.0; // Not necessarily hardcoded
byte altAddr = 40; // Let's say 40
    // Take from EEPROM during boot, set during 'g' command
bool useEEPROMalt = true; // Toogle if necessary

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
    imu.debugMode = true;
    delay(50);
    imu.setGroundAltitude(tempeAlt);

    imu.updateSensors();
    currentAlt = imu.altitude;
    previousAlt = currentAlt;

    // Temp sensor
    pinMode(23, INPUT);

    // LED
    pinMode(13, OUTPUT);

    // Restore if appropriate
    if (useEEPROMalt) {
        int temp = getInt(altAddr);
        if (temp < 10000 || temp != 0) {
            imu.setGroundAltitude((float)temp);
        }
    }
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
        heading = headingFromIMU(imu.mag.x, imu.mag.y);

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
        // coreData.storeTime(currentTime);
        // coreData.storePacketCount(packetCount);

        // Set previous stuff
        previousAlt = currentAlt;
        previousTime = currentTime;

        // Blink
        if (ledOn) {digitalWrite(13, LOW);}
        else {digitalWrite(13, HIGH);}
        ledOn = !ledOn;

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
    Serial.print("Restored time: "); Serial.println(restoredTime);
    Serial.print("Restored count: "); Serial.println(packetCount);
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
        case 'r':       // Reset everything and persist it
            restoredTime = millis();
            packetCount = 0;
            break;
        case 'g':       // Persist ground to EEPROM
            storeInt((int)currentAlt, altAddr);
            imu.setGroundAltitude(currentAlt);
            break;
        default:
            xbee.println("Invalid command");
    }
}






// ********** Temperature reading *********************************************
double readTemp(int tempPin) {
    // IF 5V, use 5000.0
    // If 3.3V, use 3300.0

    double pinMillivolts = analogRead(tempPin) * (3300.0 / 1024.0);
    double celsius = (pinMillivolts - 500.0) / 10.0; // in celsius

    // perform other calculations, or switch on a thing to use Fahrenheit
    //double fahrenheit =  celsius * 1.8 + 32;
    return celsius;
}


// Voltage reading

double readVolt(int pin) {
    double pinMillivolts = analogRead(pin) * (3300.0 / 1024.0);
    double volts = pinMillivolts / 1000.0;
    return volts * 2;
    // Or do some rubbish
    //return 6.0;
}

// Heading
int headingFromIMU(float x, float y) {
    float returnValue = 0.0;
    if (y > 0) {
        returnValue = (int)(90 - (atan(x / y)) * ( 180 / pi));
    } else if (y < 0) {
        returnValue = (int)(270 - (atan(x / y)) * (180 / pi));
    } else if (y == 0 && x < 0) {
        returnValue = 180;
    } else if (y == 0 && x > 0) {
        returnValue = 0;
    }

    return (int)(round(returnValue));
}


// Getting int from EEPROM
void storeInt(int i, int addr) {
    char two = (i & 0xFF);
    char one = ((i >> 8) & 0xFF);

    //Write the 4 bytes into the eeprom memory.
    EEPROM.write(addr, one);
    EEPROM.write(addr + 1, two);
}

int getInt(int addr) {
    int temp;

    // Bit shifting to retrieve full int
    int one = EEPROM.read(addr);
    int two = EEPROM.read(addr + 1);

    //Return the recomposed long by using bitshift.
    temp = ((two << 0) & 0xFF) + ((one << 8) & 0xFFFF);

    return temp;
}
