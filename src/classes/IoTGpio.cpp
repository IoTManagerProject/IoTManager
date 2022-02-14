#include "classes/IoTGpio.h"


IoTGpio::IoTGpio(){
    _ext1 = _ext2 = _ext3 = _ext4 = nullptr;    
}

IoTGpio::~IoTGpio(){

}


void IoTGpio::pinMode(uint8_t pin, uint8_t mode) {
    ::pinMode(pin, mode);
}

void IoTGpio::digitalWrite(uint8_t pin, uint8_t val) {
    ::digitalWrite(pin, val);
}

int IoTGpio::digitalRead(uint8_t pin) {
    return ::digitalRead(pin);
}




int IoTGpio::analogRead(uint8_t pin) {
    return ::analogRead(pin);
}

void IoTGpio::analogReference(uint8_t mode) {
    ::analogReference(mode);
}

void IoTGpio::analogWrite(uint8_t pin, int val) {
    ::analogWrite(pin, val);
}

void IoTGpio::analogWriteFreq(uint32_t freq) {
    ::analogWriteFreq(freq);
}

void IoTGpio::analogWriteRange(uint32_t range) {
    ::analogWriteRange(range);
}


void IoTGpio::regDriver(IoTGpio* newDriver) {
    
}

