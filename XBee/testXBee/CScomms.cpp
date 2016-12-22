// Implementation!


#include "CScomms.h"

CScomms::CScomms() {
    packet = " ";
    packetCount;
    tAlt = 0;
    tSpeed = 0;
    tTemp = 0;
    tPress = 0;
    tHeading = 0;
    tTime = 0;
    tState = " ";
    tVoltage = 0; // for telem
}

void CScomms::setAlt(float a) {
    tAlt = a;
}
void CScomms::setSpeed(float s) {
    tSpeed = s;
}
void CScomms::setTemp(float t) {
    tTemp = t;
}
void CScomms::setPress(float p) {
    tPress = p;
}
void CScomms::setHeading(float h) {
    tHeading = h;
}
void CScomms::setTime(float t) {
    tTime = t;
}
void CScomms::setState(String s) {
    tState = s;
}
void CScomms::setVoltage(float v) {
    tVoltage = v;
}

void CScomms::createPacket() {
    // Format: <TEAM ID>,GLIDER,<MISSION TIME>,<PACKET COUNT>,<ALT SENSOR>, <PRESSURE>,<SPEED>, <TEMP>,<VOLTAGE>,<HEADING>,<SOFTWARE STATE>, [<BONUS>]
    String s = "1234_GLIDER_";
    s += String(tTime); s += "_";
    s += String(packetCount); s += "_";
    s += String(tAlt); s += "_";
    s += String(tPress); s += "_";
    s += String(tSpeed); s += "_";
    s += String(tTemp); s += "_";
    s += String(tVoltage); s += "_";
    s += String(tHeading); s += "_";
    s += String(tState);

    packet = s;
}
