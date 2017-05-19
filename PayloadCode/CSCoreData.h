// CSDataStore
// Provide functionality for committing and retrieving values from
// EEPROM (or SD, either one works)

// Shall hold the following data:
// Current time (millis)
// Packet count

#ifndef _CSCoreData_h
#define _CSCoreData_h

#include "EEPROM.h"

class CSCoreData {
public:

    // Data

    // Public interface (use pointers?)
    void storeTime(long l);
    void storePacketCount(int i);

    long restoreTime();
    int restorePacketCount();

    void wipe();

    // Extra funcs
private:
    int timeAddress = 0;
    int packetAddress = 20;
};


#endif
