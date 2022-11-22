#include <Wire.h>
#include "utils/SerialPrint.h"

void scanI2C() {
    byte error, address;
    int nDevices;
    String message = "";
 
    nDevices = 0;
    for(address = 8; address < 127; address++ ) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
 
        if (error == 0){
            message += "I2C device found at address 0x";
            message += uint64ToString(address, 16);
            message += " !";
 
            nDevices++;
        }
        else if (error==4) {
            message += "Unknow error at address 0x";
            message += uint64ToString(address, 16);
        } 
    }
    if (nDevices == 0)
        message += "No I2C devices found\n";
    else
        message += "done\n";

    SerialPrint("i", "I2C Scaner", message);
}