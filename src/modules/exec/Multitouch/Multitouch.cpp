#include "Global.h"
#include "classes/IoTItem.h"

extern IoTGpio IoTgpio;

class Multitouch : public IoTItem
{
private:
    int _pin;
    int _int;
    int _inv;
    String _pinMode;
    int _lastButtonState = LOW;
    unsigned long _lastDebounceTime = 0;
    unsigned long timing;
    long _debounceDelay = 50;
    long _PWMDelay = 500;
    int _buttonState;
    int _reading;
    int _count = 0;
    int duration = 0;

public:
    Multitouch(String parameters) : IoTItem(parameters)
    {
        jsonRead(parameters, "pin", _pin);
        jsonRead(parameters, "pinMode", _pinMode);
        jsonRead(parameters, "debounceDelay", _debounceDelay);
        jsonRead(parameters, "PWMDelay", _PWMDelay);
        jsonRead(parameters, "int", _int);
        jsonRead(parameters, "inv", _inv);
        _round = 0;

        IoTgpio.pinMode(_pin, INPUT);
        if (_pinMode == "INPUT_PULLUP")
            IoTgpio.digitalWrite(_pin, HIGH);
        else if (_pinMode == "INPUT_PULLDOWN")
            IoTgpio.digitalWrite(_pin, LOW);

        value.valD = _buttonState = IoTgpio.digitalRead(_pin);
        // сообщаем всем о стартовом статусе без генерации события
        regEvent(_buttonState, "", false, false);
    }

    void loop()
    {
        _reading = IoTgpio.digitalRead(_pin);
        if (_reading != _lastButtonState)
        {
            _lastDebounceTime = millis();
        }

        if ((millis() - _lastDebounceTime) > _debounceDelay)
        {
            if (millis() - timing > _int && _reading == _inv && millis() - _lastDebounceTime > _PWMDelay)
            {
                timing = millis();
                duration = millis() - _lastDebounceTime - _PWMDelay;
                value.valD = duration / 50;
                regEvent(value.valD, "Multitouch");
                _count = -1;
            }

            if (_reading != _buttonState)
            {
                _buttonState = _reading;
                _count++;
                duration = 0;
            }

            if (1 < _count && millis() > _lastDebounceTime + _PWMDelay)
            {
                value.valD = _count / 2;
                regEvent(value.valD, "Multitouch");
                _count = 0;
            }
        }
        _lastButtonState = _reading;
    }

    ~Multitouch(){};
};

void *getAPI_Multitouch(String subtype, String param)
{
    if (subtype == F("Multitouch"))
    {
        return new Multitouch(param);
    }
    else
    {
        return nullptr;
    }
}