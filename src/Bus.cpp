#include "Bus.h"
#include "Class/NotAsync.h"
#include "Global.h"

void busInit() {
    myNotAsyncActions->add(
        do_BUSSCAN, [&](void*) {
            String tmp = i2c_scan();
            if (tmp == "error") {
                tmp = i2c_scan();
                Serial.println(tmp);
                jsonWriteStr(configLiveJson, "i2c", tmp);
            }
            else {
                Serial.println(tmp);
                jsonWriteStr(configLiveJson, "i2c", tmp);
            }
        },
        nullptr);
    SerialPrint("I", F("Bus"), F("Bus Init"));
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
    }
    else {
        return out;
    }
}