#include "classes/IoTGpio.h"


IoTGpio::IoTGpio(int Index) {    // указываем за какой интервал пинов будет отвечать данный экземпляр 0 - сис, 1 - 100, 2 - 200, 3 - 300, 4 -400
    index = Index;
}

IoTGpio::~IoTGpio(){

}


void IoTGpio::pinMode(uint8_t pin, uint8_t mode) {
    int pinH = pin/100;
    if (_drivers[pinH]) _drivers[pinH]->pinMode(pin - pinH*100, mode);
    else ::pinMode(pin, mode);
}

void IoTGpio::digitalWrite(uint8_t pin, uint8_t val) {
    int pinH = pin/100;
    if (_drivers[pinH]) _drivers[pinH]->digitalWrite(pin - pinH*100, val);
    else ::digitalWrite(pin, val);
}

int IoTGpio::digitalRead(uint8_t pin) {
    int pinH = pin/100;
    if (_drivers[pinH]) return _drivers[pinH]->digitalRead(pin - pinH*100);
    else return ::digitalRead(pin);
}


int IoTGpio::analogRead(uint8_t pin) {
    int pinH = pin/100;
    if (_drivers[pinH]) return _drivers[pinH]->analogRead(pin - pinH*100);
    else return ::analogRead(pin);
}

void IoTGpio::analogWrite(uint8_t pin, int val) {
    int pinH = pin/100;
    if (_drivers[pinH]) _drivers[pinH]->analogWrite(pin - pinH*100, val);
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
    _drivers[newDriver->index] = newDriver;
}