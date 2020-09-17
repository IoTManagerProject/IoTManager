#include "Bus/I2CScanner.h"
#include "Utils/PrintMessage.h"

#include <Wire.h>



I2CScanner::I2CScanner(String& out) : BusScanner(TAG_I2C, out, 2){};

void I2CScanner::init() {
    Wire.begin();
}

boolean I2CScanner::syncScan() {
    SerialPrint("I","module","scanning...");
    size_t cnt = 0;
    for (uint8_t i = 8; i < 120; i++) {
        Wire.beginTransmission(i);
        if (Wire.endTransmission() == 0) {
            SerialPrint("I","module","found: " + i);
            addResult(i, i < 119);
            cnt++;
        }
    }
    return cnt;
}