// Stephen Flores
// Implementation and whatnot

#include "CSAlt.h"

Intersema::BaroPressure_MS5607B private_baro(true);

void CSAlt::setGroundHeight(float gh) {
    groundHeight = gh;
}

void CSAlt::init() {
    private_baro.init();
}

float CSAlt::alt() {
    int altCM = private_baro.getHeightCentiMeters(); // centimeters
    return ((float)altCM) / 100.0; // meters
}

float CSAlt::altRadar() {
    float currentAlt = alt();
    return currentAlt - groundHeight;
}

float CSAlt::getGroundHeight() {
    return groundHeight;
}
