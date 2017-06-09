// CSCoreData
// Implementation file

#include "EEPROM.h"
#include "CSCoreData.h"


// Storing data to EEPROM
void CSCoreData::storeTime(long l) {
    //Serial.println("CSCoreData::storeTime()");

    //Decomposition from a long to 4 bytes by using bitshift.
      //One = Most significant -> Four = Least significant byte
      char four = (l & 0xFF);
      char three = ((l >> 8) & 0xFF);
      char two = ((l >> 16) & 0xFF);
      char one = ((l >> 24) & 0xFF);

      //Write the 4 bytes into the eeprom memory.
      EEPROM.write(timeAddress, one);
      EEPROM.write(timeAddress + 1, two);
      EEPROM.write(timeAddress + 2, three);
      EEPROM.write(timeAddress + 3, four);

    EEPROM.write(dirtyAddress, (char)25);
}

void CSCoreData::storePacketCount(int i) {
    //Serial.println("CSCoreData::storePacket()");

    char two = (i & 0xFF);
    char one = ((i >> 8) & 0xFF);

    //Write the 4 bytes into the eeprom memory.
    EEPROM.write(packetAddress, one);
    EEPROM.write(packetAddress + 1, two);

    EEPROM.write(dirtyAddress, (char)25);
}

// Restoring data from EEPROM

long CSCoreData::restoreTime() { // Verify correctness, otherwise zero
    //Serial.println("CSCoreData::restoreTime");
    long temp;

    //Read the 4 bytes from the eeprom memory.
      long one = EEPROM.read(timeAddress);
      long two = EEPROM.read(timeAddress + 1);
      long three = EEPROM.read(timeAddress + 2);
      long four = EEPROM.read(timeAddress + 3);

      //Return the recomposed long by using bitshift.
      temp = ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);

    char testValue = EEPROM.read(dirtyAddress);
    if (testValue != 25) {
        temp = 0;
    }

    return temp;
}

int CSCoreData::restorePacketCount() { // Verify correctness, otherwise zero
    //Serial.println("CSCoreData::restorePacket");
    int temp;

    // Bit shifting to retrieve full int

    int one = EEPROM.read(packetAddress);
    int two = EEPROM.read(packetAddress + 1);

    //Return the recomposed long by using bitshift.
    temp = ((two << 0) & 0xFF) + ((one << 8) & 0xFFFF);

    char testValue = EEPROM.read(dirtyAddress);
    if (testValue != 25) {
        temp = 0;
    }

    return temp;
}


// Wipe contents of EEPROM to zero (check for zero in code above)
void CSCoreData::wipe() {
    //Serial.println("CSCoreData::wipe");
    for (long i = 0; i < 100; i += 4) {
        EEPROM.write(i, 0);
    }
}
