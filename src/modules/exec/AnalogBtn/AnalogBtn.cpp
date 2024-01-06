#include "Global.h"
#include "classes/IoTItem.h"

extern IoTGpio IoTgpio;


class AnalogBtn : public IoTItem {
   private:
    int _pin, _aValue, _delta;
    int _oldVal, _newVal;

   public:
    AnalogBtn(String parameters) : IoTItem(parameters) {
        _pin = 0;
        _aValue = 0;
        _delta = 50;
        jsonRead(parameters, "pin", _pin);
        jsonRead(parameters, "aValue", _aValue);
        jsonRead(parameters, "delta", _delta);
        _round = 0;
        setInterval(-100);
    }

    void doByInterval() {
        _newVal = IoTgpio.analogRead(_pin);
        if (_aValue == -1 && _oldVal != _newVal) {
            _oldVal = _newVal;
            SerialPrint("i", "AnalogBtn", (String)_newVal);
            return;
        }

        if ((_newVal > _aValue - _delta) && (_newVal < _aValue + _delta)) {
            if (value.valD == 0) {
                value.valD = 1;
                regEvent(value.valD, "AnalogBtn");  
            }
        } else {
            if (value.valD == 1) {
                value.valD = 0;
                regEvent(value.valD, "AnalogBtn");  
            }
        }
    }

    ~AnalogBtn() {};
};

void* getAPI_AnalogBtn(String subtype, String param) {
    if (subtype == F("AnalogBtn")) {
        return new AnalogBtn(param);
    } else {
        return nullptr;
    }
}
