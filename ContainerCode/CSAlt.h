// Stephen Flores
// Implementing CSAlt code
// Barometer class
// Has access to pressure, temperature, and altitude
// Uses proprietary library (really big)

#ifndef _CSAlt
#define _CSAlt
#include "IntersemaBaro.h"

class CSAlt {
public:
    void init();
    float alt();
    float altRadar();
    void setGroundHeight(float gh);
private:
    float groundHeight = 0;
};

#endif
