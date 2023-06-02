#include "Global.h"
#include "classes/IoTItem.h"

extern IoTGpio IoTgpio;

class Impulse : public IoTItem
{
private:
    int _int;
    int _pin;
    bool _buttonState, _reading;
    bool _lastButtonState = LOW;
    unsigned long _lastDebounceTime = 0;
    int _debounceDelay = 50;
    int _count = 0;
    unsigned long timing;

public:
    Impulse(String parameters) : IoTItem(parameters)
    {
        String _pinMode;
        jsonRead(parameters, F("pin"), _pin);
        jsonRead(parameters, F("pinMode"), _pinMode);
        jsonRead(parameters, F("debounceDelay"), _debounceDelay);
        jsonRead(parameters, "int", _int);
        _round = 0;

        if (_pinMode == F("INPUT"))
            IoTgpio.pinMode(_pin, INPUT);
        else if (_pinMode == F("INPUT_PULLUP"))
            IoTgpio.pinMode(_pin, INPUT_PULLUP);
        else if (_pinMode == F("INPUT_PULLDOWN"))
        {
            IoTgpio.pinMode(_pin, INPUT);
            IoTgpio.digitalWrite(_pin, LOW);
        }

        value.valD = _buttonState = IoTgpio.digitalRead(_pin);
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
            if (_reading != _buttonState)
            {
                _buttonState = _reading;
                _count++;
            }
            if (_count == 1)
            {
                timing = millis();
            }
            if (millis() - timing > _int * 1000 && _count > 1)
            {
                timing = millis();
                value.valD = _count;
                regEvent(value.valD, F("Impulse"));
                _count = 0;
            }
        }

        _lastButtonState = _reading;
    }

    ~Impulse(){};
};

void *getAPI_Impulse(String subtype, String param)
{
    if (subtype == F("Impulse"))
    {
        return new Impulse(param);
    }
    else
    {
        return nullptr;
    }
}
