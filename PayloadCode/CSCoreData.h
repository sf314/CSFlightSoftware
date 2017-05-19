// CSDataStore
// Provide functionality for committing and retrieving values from
// EEPROM (or SD, either one works)

// Shall hold the following data:
// Current time (millis)
// Packet count

// NOTE: Dirty bit
// At address 90 there will be a special value.
// If the data at the addresses is to be trusted, that value must be 25.
// If it isn't 25, then the data must be 'dirty' (either new board, or recently
// wiped). If data is dirty, then values returned from restore() functions
// must be zero, and running store() functions sets dirty bit to 25.

// NOTE: Limits of EEPROM
// EEPROM can only store a byte. So what I can do to store a full int/long is
// break it up into sections and then store it:

// Time: 00000000  00000000  00000000  00000000 @ addr 0, 1, 2, 3
// Pack: 00000000  00000000 @ addr 4, 5

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
    int timeAddress = 0; // Stored in addr 0-3
    int packetAddress = 4; // Stored in addr 4 and 5
    int dirtyAddress = 90; //
};


#endif
