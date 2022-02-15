#include "classes/IoTGpio.h"


IoTGpio::IoTGpio(int pins) {    // инициируем стартовым числом диапазона доступных пинов для создаваемого экземпляра 0 - системный, далее 100, 200, 300, 400 или иной  
    pinNums = pins;
    _drivers[0] = nullptr;
}

IoTGpio::~IoTGpio(){

}


void IoTGpio::pinMode(uint8_t pin, uint8_t mode) {
    if (_drivers[pin/100]) _drivers[pin/100]->pinMode(pin, mode);
    else ::pinMode(pin, mode);
}

void IoTGpio::digitalWrite(uint8_t pin, uint8_t val) {
    if (_drivers[pin/100]) _drivers[pin/100]->digitalWrite(pin, val);
    else ::digitalWrite(pin, val);
}

int IoTGpio::digitalRead(uint8_t pin) {
    if (_drivers[pin/100]) return _drivers[pin/100]->digitalRead(pin);
    else return ::digitalRead(pin);
}


int IoTGpio::analogRead(uint8_t pin) {
    if (_drivers[pin/100]) return _drivers[pin/100]->analogRead(pin);
    else return ::analogRead(pin);
}

void IoTGpio::analogWrite(uint8_t pin, int val) {
    if (_drivers[pin/100]) _drivers[pin/100]->analogWrite(pin, val);
    else {
        #ifdef ESP32
            // todo: написать для esp32 аналог функции analogWrite
        #endif
        #ifdef ESP8266
            ::analogWrite(pin, val);
        #endif
    }
}


void IoTGpio::regDriver(IoTGpio* newDriver) {
    _drivers[newDriver->pinNums/100] = newDriver;
}