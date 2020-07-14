#include "Sensors/I2CScanner.h"

#include <Wire.h>

#include "StringConsts.h"
#include "PrintMessage.h"

I2CScanner::I2CScanner() : BusScanner(TAG_I2C){};

I2CScanner::~I2CScanner() {
}
bool I2CScanner::onInit() {
    Wire.begin();
    _addr = 8;
    return true;
}

void format(uint8_t addr, String& res) {
    String str = "0x";
    if (addr < 16) {
        str += "0";
    }
    str += String(addr, HEX);
    str += ", ";
    res += str;
};

boolean I2CScanner::onScan() {
    Wire.beginTransmission(_addr);
    if (Wire.endTransmission() == 0) {
        format(_addr, _results);
    }
    return ++_addr > 120;
}