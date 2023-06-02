#include "Global.h"
#include "classes/IoTItem.h"

extern IoTGpio IoTgpio;


class ButtonIn : public IoTItem {
   private:
    int _pin;
    bool _execLevel, _fixState, _inv, _buttonState, _reading;
    bool _lastButtonState = LOW;
    unsigned long _lastDebounceTime = 0;
    int _debounceDelay = 50;

   public:
    ButtonIn(String parameters): IoTItem(parameters) {
        String _pinMode;

        jsonRead(parameters, F("inv"), _inv);
        jsonRead(parameters, F("pin"), _pin);
        jsonRead(parameters, F("execLevel"), _execLevel);
        jsonRead(parameters, F("pinMode"), _pinMode);
        jsonRead(parameters, F("debounceDelay"), _debounceDelay);
        jsonRead(parameters, F("fixState"), _fixState);
        _round = 0;
        
        if (_pinMode == F("INPUT")) IoTgpio.pinMode(_pin, INPUT);
        else if (_pinMode == F("INPUT_PULLUP")) IoTgpio.pinMode(_pin, INPUT_PULLUP);
        else if (_pinMode == F("INPUT_PULLDOWN")) {IoTgpio.pinMode(_pin, INPUT); IoTgpio.digitalWrite(_pin, LOW);}
        
        value.valD = _buttonState = IoTgpio.digitalRead(_pin);
        if (_inv) value.valD = _buttonState = !_buttonState;    // инвертируем, если нужно показания
        // сообщаем всем о стартовом статусе без генерации события
        regEvent(_buttonState, "", false, false);
    }

    void loop() {
        _reading = IoTgpio.digitalRead(_pin);
        if (_reading != _lastButtonState) {
            // reset the debouncing timer
            _lastDebounceTime = millis();
        }

        if ((millis() - _lastDebounceTime) > _debounceDelay) {
            if (_reading != _buttonState) {
               _buttonState = _reading;

                if (_fixState == 1 && _buttonState == _execLevel) {
                    value.valD = !value.valD;
                    regEvent(value.valD, F("ButtonIn")); 
                }
                
                if (_fixState == 0) {
                    value.valD = _buttonState;
                    if (_inv) value.valD = !_buttonState;    // инвертируем, если нужно показания
                    regEvent(value.valD, F("ButtonIn"));
                }
            }
        }

        _lastButtonState = _reading;
    }

    void setValue(const IoTValue& Value, bool genEvent = true) {
        value = Value;
        regEvent((String)(int)value.valD, F("ButtonIn"), false, genEvent);
    }

    String getValue() {
        return (String)(int)value.valD;
    }

    ~ButtonIn() {};
};

void* getAPI_ButtonIn(String subtype, String param) {
    if (subtype == F("ButtonIn")) {
        return new ButtonIn(param);
    } else {
        return nullptr;
    }
}
