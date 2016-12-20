// Class for easily writing to the SD card
// Start loop with new line
// Append contents to the file with OBJ.log()

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

class CSlog {
public:
    CSlog();

    void initSD();s
    void startNewLog();
    void setupFile(String name, String headerString);
    void setSDPin();
    
    void logInt(int i); // Each type gets one
    void logDouble(float f);
    void logString(String s);
    void logBool(bool b);
private:
    int sdPin;
    int logNumber;
    File dataFile;
    String fileName;
};
