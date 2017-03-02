// Stephen Flores
// Implement CSLog class (basically ASCEND_Data funcs)
// Remember: filename and headers must be set at top of CSLog.h!!!!
// Also remember: sd pin must also be set in CSLog.h!!!!


#include "CSLog.h"

void CSLog::add(float f) {
    SD_add(f);
}

void CSLog::add(int i) {
    SD_add(i);
}

void CSLog::saveTelemetry() {
    SD_save();
}

void CSLog::init() {
    SD_init();
}

bool CSLog::available() {
    return SD_available();
}
