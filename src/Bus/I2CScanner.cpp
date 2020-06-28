#include "Bus/I2CScanner.h"
#include "Utils/PrintMessage.h"

#include <Wire.h>

static const char* MODULE = "I2C";

I2CScanner::I2CScanner(String& out) : BusScanner(TAG_I2C, out, 2){};

void I2CScanner::init() {
    Wire.begin();
}

boolean I2CScanner::syncScan() {
    pm.info(String("scanning..."));
    size_t cnt = 0;
    for (uint8_t i = 8; i < 120; i++) {
        Wire.beginTransmission(i);
        if (Wire.endTransmission() == 0) {
            pm.info("found: " + String(i, DEC));
            addResult(i, i < 119);
            cnt++;
        }
    }
    return cnt;
}