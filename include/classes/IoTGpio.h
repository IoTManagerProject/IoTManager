#pragma once
#include <Arduino.h>

class IoTGpio {
    public:
        IoTGpio(int pins);
        ~IoTGpio();

        virtual void pinMode(int pin, uint8_t mode);
        virtual void digitalWrite(int pin, uint8_t val);
        virtual int digitalRead(int pin);
        virtual int analogRead(int pin);
        virtual void analogWrite(int pin, int val);
        virtual void digitalInvert(int pin);

        int index;
        void regDriver(IoTGpio* newDriver);
        void clearDrivers();

    private:
        IoTGpio* _drivers[5] = {nullptr};  //ссылки на объекты доступа к портам более 100, 200, 300, 400. Нулевой элемент используется как маркер - и возвращается nullptr при обращении

};