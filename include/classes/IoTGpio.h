#pragma once
#include <Arduino.h>

class IoTGpio {
    public:
        IoTGpio(int pins);
        ~IoTGpio();

        void pinMode(uint8_t pin, uint8_t mode);
        void digitalWrite(uint8_t pin, uint8_t val);
        int digitalRead(uint8_t pin);
        int analogRead(uint8_t pin);
        void analogWrite(uint8_t pin, int val);

        int pinNums;    // база для определения диапазона номеров пинов. pinNums + используемый
        void regDriver(IoTGpio* newDriver);

    private:
        IoTGpio* _drivers[5];  //ссылки на объекты доступа к портам более 100, 200, 300, 400. Нулевой элемент используется как маркер - и возвращается nullptr при обращении

};