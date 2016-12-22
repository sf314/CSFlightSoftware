// Library for simplified XBee communications, and implementation of comms protocol.
/*
Desired flow:
CScomms comms;
(Setup)
comms.init();
(Loop)
- get data
- create telemetry packet
- log telemetry to SD (include all telemetry)
- send telemetry
    - if on the ground, play a sound and stop stuff
    - if (other stuff)
    - emergency deploy?
- read command from ground (if any)
- execute command
- wait 1 sec

Containter flow:
(Loop)
- get data
- create telemetry packet
- log telemetry to SD (all the things!);
- send telemetry
    - if 2s after release, stop and play sound
    - if other stuff, do things
- read command (emergency deploy?)
- execute command
- wait 1 sec
*/

// Telemetry: <TEAM ID>,GLIDER,<MISSION TIME>,<PACKET COUNT>,<ALT SENSOR>, <PRESSURE>,<SPEED>, <TEMP>,<VOLTAGE>,<HEADING>,<SOFTWARE STATE>, [<BONUS>]

class CScomms {
public:
    CScomms();
    
    sendTelemetry();
    readCommand();

    void setAlt();           // Put all this stuff in a single function in main code
    void setSpeed();
    void setTemp();
    void setPress();
    void setHeading();
    void setTime();
    void setState();
    void setVoltage();

private:
    Xbee xbee;
    String packet;

    // Telemetry data:
    int packetCount;
    float tAlt;
    float tSpeed;
    float tTemp;
    float tPress;
    float tHeading;
    float tTime;
    String tState;
    float tVoltage; // for telem

    void createPacket();
}
