// Cansat 2017 Container software
// Must access CSAlt, CSComms, and CSLog
// Requirements:
// Transmit telemetry at 1Hz (rq. fields in req. format)
// Deploy payload (via payload release function) at 400m radar alt
// Activate beacon and stop telemetry 2s after deploying payload
// Telemetry complete!

// NOTE: ***** PINOUT *****
/*
Altimeter:
    SCL -> Pin 19 (SCL)
    SDA -> Pin 18 (SDA)
Temp sensor:
    Data -> Pin 22 (A8)
Nichrome:
    Cut -> Pin 11
XBee:
    TX/DOUT -> Pin 0 (RX)
    RX/DIN -> Pin 1 (TX)
Buzzer:
    Vin -> Pin 6 (PWM)
*/

// NOTE: *** EEPROM addresses ***
/*
altitude @ 0 (stored as int, rounds to nearest)
state at 1
packet count at 2
time at 3
*/

// NOTE:
// Container does not need SD!
// Need to include temp sensor on payload
// Stop telemetry 2s after deployment
// Currently, mission time is not persisted. Reboots reset it to zero.

// TODO:
// Stop telemetry 2 seconds after deployment


// ********** Required Libraries **********************************************
#include "Wire.h"
#include "IntersemaBaro.h"
#include "CSAlt.h"
#include "CSTemp.h"
#include "CSBuzzer.h"
#include "CSVolt.h"
#include <EEPROM.h>
#include "SoftwareSerial.h"
//#include "CSCoreData.h"

// ********** Objects *********************************************************
CSAlt baro;
CSTemp temp;
CSBuzzer buzzer; bool playBuzzer = true; bool buzzerIsOn = false;
CSVolt volt;
SoftwareSerial xbee(0, 1);

int led = 13;
int nichromePin = 11;
bool on = false;
String dataString = "";
float tempe = 344.0; // Custom launchpads here if desired

// ********** Control variables ***********************************************
int state                 = 0;
    const int boot        = 0;
    const int launchpad   = 1;
    const int ascent      = 2;
    const int descent     = 3;
    const int deploy      = 4;

// ********** Time variables **************************************************
long currentTime    = 0;
long previousTime   = 0;

long  startTime     = 0;
long restoredTime   = 0;

#define refreshRate   1000

long ledPrevTime    = 0;
int ledBlinkRate    = 1000;

// ********** Telemetry structure *********************************************
float currentAlt    = 0;
float previousAlt   = 0;
float verticalSpeed = 0;
float currentTemp   = 0;
float currentVolt   = 0;
int packetCount     = 1;

// ********** Core Data structure *********************************************
int CSCoreData_timeAddress = 0;
int CSCoreData_altAddress = 4;
int CSCoreData_packetAddress = 6;
void CSCoreData_storeLong(int addr, long l); // and func prototypes
void CSCoreData_storeInt(int addr, int i);
long CSCoreData_restoreLong(int addr);
int CSCoreData_restoreInt(int addr);

// ********** State change conditions *****************************************
#define targetAltitude          405
#define ascentVSpeedThreshold   1
#define ascentAltThreshold      40
#define descentVSpeedThreshold  -1
#define forceDeployThreshold    150
#define noVSpeedThreshold       1
#define cutTime                 4
#define cutoffAlt               100
#define landedAltThreshold      50

// ********** Setup Function *********************************************#####
void setup() {
    // Startup procedure
    Serial.begin(9600);
    Wire.begin();
    pinMode(led, OUTPUT);
    delay(1000);

    // Restore values from reboot
    restore();

    // * Barometer
    Serial.println("Initializing barometer");
    baro.init();
    currentAlt = baro.altRadar(); // Initialize alt stuff (GH restored)
    previousAlt = currentAlt;

    // * Temperature
    temp = CSTemp(22, 3300);

    // * Nichrome wire
    pinMode(nichromePin, OUTPUT);

    // * Buzzer
    buzzer = CSBuzzer(12);
    buzzer.setFrequency(800);

    // * Voltage divider
    volt = CSVolt(14);

    // * CSComms
    CSComms_begin(9600);
    CSComms_log("Reboot");

    // * Time
    Serial.println("Setting start time");
    startTime = setStartTime();
}

// ********** Loop Function **********************************************#####
void loop() {
    currentTime = restoredTime + millis() - startTime;

    // *** Non-time triggered functions. Run every time. **********************

    // * Receive command
    if (CSComms_available()) {
        char c = CSComms_read();
        Serial.println("Received: " + String(c));

        CSComms_parse(c);
    }

    if (Serial.available()) {
        char c = Serial.read();
        Serial.println("Received: " + String(c));

        CSComms_parse(c);
    }

    // *** Time-triggered functions. Must happen on every iteration ***********
    if (currentTime - previousTime > refreshRate) {
        // Switch on state
        switch (state) {
            case boot:
                boot_f(); break;
            case launchpad:
                launchpad_f(); break;
            case ascent:
                ascent_f(); break;
            case descent:
                descent_f(); break;
            case deploy:
                deploy_f(); break;
            default:
                Serial.println(F("Error: invalid state"));
                state = 0;
        }

        // * Print data


        // Set previous variables
        previousTime = currentTime;
        previousAlt = currentAlt;
        store();
    }

    // * Blink the led for [blink rate]
    if (currentTime - ledPrevTime > ledBlinkRate) {
        if (on) {
            digitalWrite(led, LOW); on = false;
        } else {
            digitalWrite(led, HIGH); on = true;
        }
        ledPrevTime = currentTime;
    }

}






// ********** Miscellaneous functions *****************************************
long setStartTime() {
    // TODO: Recover time t from file if it exists
    // Prev time already restored during setup();
    return millis(); // do something smarter(?) eventually
}



// ********** State Functions *************************************************
void boot_f() {
    // Restore data from EEPROM.
    // Done in setup already?
    //restore();
    CSComms_log("Boot success");
    state = launchpad;
}

void launchpad_f() {
    updateTelemetry();
    transmitTelemetry();

    if (currentAlt > ascentAltThreshold) {
        state = ascent;
        delay(1000); // Don't let noise ruin it
    }

    ledBlinkRate = 1000;
}

void ascent_f() {
    updateTelemetry();
    transmitTelemetry();

    if (verticalSpeed < descentVSpeedThreshold) {
        state = descent;
    }

    ledBlinkRate = 1000;
}

void descent_f() {
    updateTelemetry();
    transmitTelemetry();

    if (currentAlt < targetAltitude) {
        ledBlinkRate = 250;

        // Start cutting sequence
        startCut();

        // Wait n-seconds, keep transmitting telemetry but only for 2 seconds
        for (int i = 0; i < cutTime; i++) {
            state = deploy;
            if (i <= 2) {
                updateTelemetry();
                transmitTelemetry();
            } else {
                // Don't send stuff
            }
            delay(1000);
        }

        stopCut();

        state = deploy;
    }

    ledBlinkRate = 1000;
}

void deploy_f() {
    // NOTE: Don't send telemetry when deployed
    updateTelemetry();
    transmitTelemetry();

    // Play buzzer
    if (currentAlt < landedAltThreshold) {
        if (buzzerIsOn) {
            digitalWrite(12, LOW);
            buzzerIsOn = false;
        } else {
            if (playBuzzer) {
                digitalWrite(12, HIGH);
            }
            buzzerIsOn = true;
        }
    }
}


// ********** Implement CSComms functions *************************************
void CSComms_add(long l) {
    dataString += String(l) + ",";
}

void CSComms_add(float f) {
    dataString += String(f) + ",";
}

void CSComms_add(String s) {
    dataString += s;
}

void CSComms_log(String s) {
    xbee.println(s);
}

void CSComms_transmit() {
    xbee.println(dataString);
    dataString = "";
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
        case 'x': // Auto cut command
            //nichrome.start(currentTime, cutTime);
            digitalWrite(11, HIGH); // Start cut

            // Wait n-seconds, keep transmitting telemetry but only for 2 seconds
            for (int i = 0; i < cutTime; i++) {
                state = deploy;
                if (i <= 2) {
                    updateTelemetry();
                    transmitTelemetry();
                } else {
                    // Don't send stuff
                }
                delay(1000);
            }

            digitalWrite(11, LOW); // Stop cut

            state = deploy;
            ledBlinkRate = 250;
            break;
        case '0': // switch to state 0 (reboot)
            state = boot; break;
        case '1': // switch to launchpad
            state = launchpad; break;
        case '2': // switch to ascent
            state = ascent; break;
        case '3': //  switch to descent
            state = descent; break;
        case '4': // switch to deploy
            state = deploy; break;
        case 'g': // Set ground height from current (use on launchpad) (and persist it)
            baro.setGroundHeight(baro.alt());
            currentAlt = baro.altRadar();
            previousAlt = currentAlt;
            store();
            Serial.println("Saving altitude: " + String(currentAlt));
            break;
        case 'c': // Reset packet count to 1 and persist it
            packetCount = 1;
            store();
            break;
        case 'p': // Print current persisted info
            CSComms_log("St: " + String(currentTime) + ", PC: " + String(packetCount) + ", GH: " + String(baro.getGroundHeight()));
            break;
        case 'r':
            Serial.println("Reset value");
            restoredTime = 0;
            startTime = millis();
            packetCount = 0;
            currentTime = 0;
            previousTime = 0;
            store();
            break;
        case 'b':
            // Toggle the buzzer (if it gets annoying)
            playBuzzer = !playBuzzer;
            break;
        default:
            Serial.println("Invalid command char");
    }
}




// ********** Implement data update and transmit functions ********************
void updateTelemetry() {
    currentAlt = baro.altRadar();
    verticalSpeed = (currentAlt - previousAlt) / ((currentTime - previousTime) / 1000); // m/s
    currentTemp = temp.read();
    currentVolt = volt.read();
}

void transmitTelemetry() {
    CSComms_add("2848,CONTAINER,");
    CSComms_add(((float)currentTime / 1000.0));
    CSComms_add((long)packetCount);
    CSComms_add(currentAlt);
    CSComms_add(currentTemp);
    CSComms_add(currentVolt);
    CSComms_add((long)state);
    Serial.println(dataString);
    CSComms_transmit();
}



// ********** Cut nichrome functions ******************************************
void startCut() {
    digitalWrite(nichromePin, HIGH);
}

void stopCut() {
    digitalWrite(nichromePin, LOW);
}




// ********** EEPROM Persistence functions ************************************
void store() {
    // Time, alt, packet count
    CSCoreData_storeLong(CSCoreData_timeAddress, currentTime);
    CSCoreData_storeInt(CSCoreData_altAddress, (int)baro.getGroundHeight());
    CSCoreData_storeInt(CSCoreData_packetAddress, packetCount);
}

void restore() {
    //CSComms_log("Restoring from EEPROM");
    // int addr = 0;
    // state = EEPROM.read(addr);
    // addr += sizeof(int);
    // packetCount = EEPROM.read(addr);
    // addr += sizeof(int);
    // baro.setGroundHeight((float)EEPROM.read(addr));
    // addr += sizeof(int);
    // restoredTime = EEPROM.read(addr);

    restoredTime = CSCoreData_restoreLong(CSCoreData_timeAddress);
    baro.setGroundHeight((float)CSCoreData_restoreInt(CSCoreData_altAddress));
    packetCount = CSCoreData_restoreInt(CSCoreData_packetAddress);
}



// ********** Core Data implementation ****************************************

void CSCoreData_storeLong(int addr, long l) {
    //Decomposition from a long to 4 bytes by using bitshift.
      //One = Most significant -> Four = Least significant byte
      char four = (l & 0xFF);
      char three = ((l >> 8) & 0xFF);
      char two = ((l >> 16) & 0xFF);
      char one = ((l >> 24) & 0xFF);

      //Write the 4 bytes into the eeprom memory.
      EEPROM.write(addr, one);
      EEPROM.write(addr + 1, two);
      EEPROM.write(addr + 2, three);
      EEPROM.write(addr + 3, four);

    //EEPROM.write(dirtyAddress, (char)25);
}

void CSCoreData_storeInt(int addr, int i) {
    char two = (i & 0xFF);
    char one = ((i >> 8) & 0xFF);

    //Write the 4 bytes into the eeprom memory.
    EEPROM.write(addr, one);
    EEPROM.write(addr + 1, two);

    //EEPROM.write(dirtyAddress, (char)25);

}

long CSCoreData_restoreLong(int addr) {
    long temp;

    //Read the 4 bytes from the eeprom memory.
      long one = EEPROM.read(addr);
      long two = EEPROM.read(addr + 1);
      long three = EEPROM.read(addr + 2);
      long four = EEPROM.read(addr + 3);

      //Return the recomposed long by using bitshift.
      temp = ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);

    return temp;
}

int CSCoreData_restoreInt(int addr) {
    int temp;

    // Bit shifting to retrieve full int

    int one = EEPROM.read(addr);
    int two = EEPROM.read(addr + 1);

    //Return the recomposed long by using bitshift.
    temp = ((two << 0) & 0xFF) + ((one << 8) & 0xFFFF);

    return temp;
}
