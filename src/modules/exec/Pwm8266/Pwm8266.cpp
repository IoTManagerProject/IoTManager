#include "Global.h"
#include "classes/IoTItem.h"
#include "Arduino.h"

extern IoTGpio IoTgpio;

class Pwm8266 : public IoTItem {
   private:
    int _pin;
    int _freq; 
    int _apin, _oldValue;
    bool _freezVal = true;

   public:
    Pwm8266(String parameters): IoTItem(parameters) {
        _interval = _interval / 1000;   // корректируем величину интервала int, теперь он в миллисекундах   

        jsonRead(parameters, "pin", _pin);
        jsonRead(parameters, "freq", _freq);
        
        IoTgpio.pinMode(_pin, OUTPUT);
        analogWriteFreq(_freq);
        analogWriteResolution(10);
        IoTgpio.analogWrite(_pin, value.valD);

        jsonRead(parameters, "apin", _apin);
        if (_apin >= 0) IoTgpio.pinMode(_apin, INPUT);
    }

    void doByInterval() {
        if (_apin >= 0) {
            value.valD = IoTgpio.analogRead(_apin);
            if (value.valD > 1018) value.valD = 1024;  // нивелируем погрешность установки мин и макс
                else if (value.valD < 9) value.valD = 0;   // из-за смягчения значений
            if (abs(_oldValue - value.valD) > 5) {
                _oldValue = value.valD;
                analogWrite(_pin,value.valD);
                _freezVal = false;
            } else {
                if (!_freezVal) {   // отправляем событие только раз после серии изменений, чтоб не спамить события
                    regEvent(value.valD, "Pwm8266");
                    _freezVal = true;
                }
            }
        }
    }
 
    void setValue(const IoTValue& Value, bool genEvent = true) {
        value = Value;
        IoTgpio.analogWrite(_pin, value.valD);
        regEvent(value.valD, "Pwm8266", false, genEvent);
    }
    //=======================================================================================================

    ~Pwm8266() {};
};


void* getAPI_Pwm8266(String subtype, String param) {
    if (subtype == F("Pwm8266")) {
        return new Pwm8266(param);
    } else {
        return nullptr;
    }
}
