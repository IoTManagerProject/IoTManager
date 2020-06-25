#include "Utils/i2c_bus.h"

#include "Global.h"

void do_i2c_scanning() {
    String tmp = i2c_scan();
    if (tmp == "error") {
        tmp = i2c_scan();
    }
    jsonWriteStr(configLiveJson, "i2c", tmp);
}

const String i2c_scan() {
    String out;
    byte count = 0;
    Wire.begin();
    for (byte i = 8; i < 120; i++) {
        Wire.beginTransmission(i);
        if (Wire.endTransmission() == 0) {
            count++;
            out += String(count) + ". 0x" + String(i, HEX) + "; ";
            delay(1);
        }
    }
    if (count == 0) {
        return "error";
    } else {
        return out;
    }
}