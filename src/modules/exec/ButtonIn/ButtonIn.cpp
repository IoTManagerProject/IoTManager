#include "Global.h"
#include "classes/IoTItem.h"

extern IoTGpio IoTgpio;


class ButtonIn : public IoTItem {
   private:
    int _pin;
    bool _execLevel;
    int _fixState;
    String _pinMode;
    int _lastButtonState = LOW;
    unsigned long _lastDebounceTime = 0;
    long _debounceDelay = 50;
    int _buttonState;
    int _reading;

   public:
    ButtonIn(String parameters): IoTItem(parameters) {
        jsonRead(parameters, "pin", _pin);
        jsonRead(parameters, "execLevel", _execLevel);
        jsonRead(parameters, "pinMode", _pinMode);
        jsonRead(parameters, "debounceDelay", _debounceDelay);
        jsonRead(parameters, "fixState", _fixState);
        _round = 0;
        //Serial.printf("vvvvvvvvvvvvvvvv =%d \n", _fixState);
        
        IoTgpio.pinMode(_pin, INPUT);
        if (_pinMode == "INPUT_PULLUP") IoTgpio.digitalWrite(_pin, HIGH);
        else if (_pinMode == "INPUT_PULLDOWN") IoTgpio.digitalWrite(_pin, LOW);

        value.valD = _buttonState = IoTgpio.digitalRead(_pin);
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
                    regEvent(value.valD, "ButtonIn"); 
                }

                if (_fixState == 2) {
                    value.valD = !value.valD;
                    regEvent(value.valD, "ButtonIn"); 
                }
                
                if (_fixState == 0) {
                    value.valD = _buttonState;
                    regEvent(value.valD, "ButtonIn");
                }
            }
        }

        _lastButtonState = _reading;
    }

    void setValue(const IoTValue& Value, bool genEvent = true) {
        value = Value;
        regEvent((String)(int)value.valD, "ButtonIn", false, genEvent);
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
