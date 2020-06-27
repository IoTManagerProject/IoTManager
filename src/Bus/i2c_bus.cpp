#include "Bus/BusScanner.h"

#include <Wire.h>
#include "Utils/PrintMessage.h"

static const char* MODULE = "I2C";

boolean I2CScanner::syncScan() {
    Wire.begin();
    pm.info("scanning i2c...");
    size_t cnt = 0;
    for (uint8_t i = 8; i < 120; i++) {
        Wire.beginTransmission(i);
        if (Wire.endTransmission() == 0) {
            pm.info("found device: " + i);
            addResult(i, i < 119);
            cnt++;
        }
    }
    if (!cnt) { 
        addResult("не найдено");
    }
    return cnt;
}