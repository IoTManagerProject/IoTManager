#include "Global.h"

void do_i2c_scanning() {
    if (i2c_scanning) {
        i2c_scanning = false;
        String tmp = i2c_scan();
        if (tmp == "error") {
            tmp = i2c_scan();
            Serial.println(tmp);
            jsonWriteStr(configJson, "i2c", tmp);
        } else {
            Serial.println(tmp);
            jsonWriteStr(configJson, "i2c", tmp);
        }
    }
}

String i2c_scan() {
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
