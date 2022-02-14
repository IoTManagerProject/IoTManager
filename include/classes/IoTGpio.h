#pragma once
#include <Arduino.h>

class IoTGpio {
    public:
        IoTGpio();
        ~IoTGpio();

        void pinMode(uint8_t pin, uint8_t mode);
        void digitalWrite(uint8_t pin, uint8_t val);
        int digitalRead(uint8_t pin);
        int analogRead(uint8_t pin);
        void analogReference(uint8_t mode);
        void analogWrite(uint8_t pin, int val);
        void analogWriteFreq(uint32_t freq);
        void analogWriteRange(uint32_t range);

        void regDriver(IoTGpio* newDriver);

    private:
        IoTGpio* _ext1;    //ссылка на объект доступа к портам свыше 100
        IoTGpio* _ext2;    //200 
        IoTGpio* _ext3;    //300
        IoTGpio* _ext4;    //400
};