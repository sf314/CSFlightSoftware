// For handling the pitot tube

#include <Wire.h>
#include <Arduino.h>

// Global variables (will be returned)
float CSPitot_velocity = 0.0;
byte CSPitot_addr = 0x28;

// Predeclaration:
//byte CSPitot_fetch_pressure(unsigned int *p_Pressure);
#define TRUE 1
#define FALSE 0
#define isDebug 1

// Local Vars
byte CSPitot_status;
unsigned int P_dat, T_dat;
double PR, PR1, TR, V, VV, CSPitot_density;

// Debug to console
void debug(String s) {
    if (isDebug) {
        Serial.println("CSPitot: " + s);
    }
}

// Clearly set address
void CSPitot_setAddress(byte b) {
    CSPitot_addr = b;
}


// Example fetch_pressure

byte CSPitot_fetch_pressure(unsigned int *p_P_dat, unsigned int *p_T_dat) {
    debug("Fetch Pressure");

  byte address, Press_H, Press_L, _status;
  unsigned int P_dat;
  unsigned int T_dat;

  debug("First Wire encounter");
  address = CSPitot_addr;
  debug("\tBegin Transmission");
  Wire.beginTransmission(address);
  debug("\tEnd Transmission");
  int error = Wire.endTransmission();
  delay(100);

  if (error != 0) { // Break early if error on I2C
      Serial.println("Breaking early");
      return 0;
  }

  debug("Second Wire encounter");
  debug("\tRequest");
  Wire.requestFrom(address, 4);//Request 4 bytes need 4 bytes are read
  debug("\tReads");
  Press_H = Wire.read();
  Press_L = Wire.read();
  byte Temp_H = Wire.read();
  byte  Temp_L = Wire.read();
  debug("\tEnd");
  Wire.endTransmission();

  debug("End of Wire");

  _status = (Press_H >> 6) & 0x03;
  Press_H = Press_H & 0x3f;
  P_dat = (((unsigned int)Press_H) << 8) | Press_L;
  *p_P_dat = P_dat;

  Temp_L = (Temp_L >> 5);
  T_dat = (((unsigned int)Temp_H) << 3) | Temp_L;
  *p_T_dat = T_dat;
  return (_status);
}


// Fetch new value
void CSPitot_updateValues() {
    debug("Update values");
    // Check status
    CSPitot_status = CSPitot_fetch_pressure(&P_dat, &T_dat);
    switch (CSPitot_status) {
        case 0: Serial.println("   ");
          break;
        case 1: Serial.println("Busy");
          break;
        case 2: Serial.println("Slate");
          break;
        default: Serial.println("Error");
          break;
    }

    // * Set values (from example)
    TR = (double)((T_dat * 0.09770395701));
    TR = TR - 50;

    //PR1 = (double)((P_dat-819.15)/(14744.7)) ;
    PR1 = (double)((P_dat-819.15)/(15000));
    PR = (PR1 - 0.49060678) ;
    //PR = (double)((P_dat-14)*13106+1638.3);
    PR = abs(PR)*6894.76; // Converting PSI to Pa

    CSPitot_density = PR / (287 * TR);

    V = (2 * PR / 1.22); // V^2 [m^2 / s^2]
    VV = (sqrt(V)); // For some reason

    CSPitot_velocity = VV;

}



// ********** This is the bit you'll use

float CSPitot_getVelocity() {
    debug("Get Velocity");

    Wire.begin();
    // Fetch new data
    CSPitot_updateValues();

    // Return it
    return CSPitot_velocity;
}
