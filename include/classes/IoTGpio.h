#pragma once
#include <Arduino.h>

class IoTGpio {
    public:
        IoTGpio(int pins);
        ~IoTGpio();

        virtual void pinMode(uint8_t pin, uint8_t mode);
        virtual void digitalWrite(uint8_t pin, uint8_t val);
        virtual int digitalRead(uint8_t pin);
        virtual int analogRead(uint8_t pin);
        virtual void analogWrite(uint8_t pin, int val);
        virtual void digitalInvert(uint8_t pin);

        int index;
        void regDriver(IoTGpio* newDriver);
        void clearDrivers();

    private:
        IoTGpio* _drivers[5] = {nullptr};  //ссылки на объекты доступа к портам более 100, 200, 300, 400. Нулевой элемент используется как маркер - и возвращается nullptr при обращении

};