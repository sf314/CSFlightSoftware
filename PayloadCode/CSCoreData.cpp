// CSCoreData
// Implementation file

#include "EEPROM.h"
#include "CSCoreData.h"


// Storing data to EEPROM
void CSCoreData::storeTime(long l) {
    EEPROM.write(timeAddress, l);
}

void CSCoreData::storePacketCount(int i) {
    EEPROM.write(packetAddress, i);
}

// Restoring data from EEPROM

long CSCoreData::restoreTime() { // Verify correctness, otherwise zero
    long temp = EEPROM.read(timeAddress);
    // Check?
    return temp;
}

int CSCoreData::restorePacketCount() { // Verify correctness, otherwise zero
    int temp = EEPROM.read(packetAddress);
    // Check?
    return temp;
}


// Wipe contents of EEPROM to zero (check for zero in code above)
void CSCoreData::wipe() {
    for (long i = 0; i < 100; i += 4) {
        EEPROM.write(i, 0);
    }
}
