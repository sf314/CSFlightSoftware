// Stephen Flores
// CSLog.h
// Use log object to save to SD card

// Basically pass-through library for ASCEND_Data funcs.

// ********** ASCEND_Data.h ***************************************************
// Stephen Flores
// SDWrite.h
// Created 2017/2/2
// Function-driven SDWrite

#ifndef _CSLog_h
#define _CSLog_h

#include "SPI.h"
#include "SD.h"

// DEFINITIONS: EDIT THESE
#define filename "test.csv"
#define headers "Time, altRadar"
#define sdPin 10


// VARIABLES
File dataFile;
String dataString;

// ERROR PRINTING

// DATA MANIPULATION
void SD_add(int i) {
    dataString += String(i) + ", ";
}

void SD_add(String s) {
    dataString += s + ", ";
}

void SD_add(float f) {
    dataString += String(f) + ", ";
}

void SD_add(long l) {
    dataString += String(l) + ", ";
}

template<size_t N>
void SD_add(float (&arr)[N]) {
    //int size = sizeof(arr) / sizeof(arr[0]);
    size_t size = N;
    //Serial.println("array size" + String(N));
    for (int i = 0; i < size; i++) {
        dataString += String(arr[i]) + ", ";
    }
}

void SD_save() {
    dataFile = SD.open(filename, FILE_WRITE);
    if (dataFile) {
        dataFile.println(dataString);
        dataString = "";
        dataFile.close();
    }
}

bool SD_available() {
    dataFile = SD.open(filename);
    if (dataFile) {
        dataFile.close();
        return true;
    } else {
        Serial.println("\t\t\t*** Error: cannot access SD ***");
        return false;
    }
}


// INITIALIZER
void SD_init() {
    // *** SD setup:
        Serial.println("Initializing SD card...");

        int tries = 3;
        while (!SD.begin(sdPin) && tries > 0) {
            Serial.print(".");
            tries--;
            delay(1000);
        }
        Serial.println("Attempted initialization on pin " + String(sdPin));

        dataFile = SD.open(filename, FILE_WRITE);
        if (dataFile) {
            Serial.println("Writing SD headers...");
            dataFile.println("Time, t1, t2");
            dataFile.close();
        } else {
            Serial.println("\t\t\t*** Error: cannot access SD ***");
        }

        dataFile = SD.open(filename);
        Serial.println("Checking if file is accessible...");
        delay(100);
        if (dataFile) {
            Serial.println("File is accessible");
            dataFile.close();
        } else {
            Serial.println("\t\t\t*** Error: cannot access SD ***");
        }
}


// ********** End ASCEND_Data.h ***********************************************



// ********** CSLog.h *********************************************************

class CSLog {
public:
    void add(float f);
    void add(int i);
    void add(long l);
    //void add(float arr[]); // for arrays, use SD_add(myArr); in main.
    void saveTelemetry();
    bool available();
    void init();
};



// ********** CSLog implementation ********************************************
void CSLog::add(float f) {
    SD_add(f);
}

void CSLog::add(int i) {
    SD_add(i);
}

void CSLog::add(long l) {
    SD_add(l);
}

void CSLog::saveTelemetry() {
    SD_save();
}

void CSLog::init() {
    SD_init();
}

bool CSLog::available() {
    return SD_available();
    //return true;
}


#endif
