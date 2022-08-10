#include "Global.h"
#include "classes/IoTItem.h"
#include "classes/IoTGpio.h"

extern IoTGpio IoTgpio;

#include <Servo.h>

class IoTServo : public IoTItem {
    private:
        Servo servObj;
        int _apin, _oldValue;
        int _locmap1, _locmap2, _locmap3, _locmap4;

    public:
        IoTServo(String parameters): IoTItem(parameters) {
            int pin;
            jsonRead(parameters, "pin", pin);
            servObj.attach(pin);

            jsonRead(parameters, "apin", _apin);
            if (_apin >= 0) IoTgpio.pinMode(_apin, INPUT);
        
            String map;
            jsonRead(parameters, F("amap"), map, false);
            if (map != "") {
                _locmap1 = selectFromMarkerToMarker(map, ",", 0).toInt();
                _locmap2 = selectFromMarkerToMarker(map, ",", 1).toInt();
                _locmap3 = selectFromMarkerToMarker(map, ",", 2).toInt();
                _locmap4 = selectFromMarkerToMarker(map, ",", 3).toInt();
            }
        }

        void doByInterval() {
            if (_apin >= 0) {
                value.valD = map(IoTgpio.analogRead(_apin), _locmap1, _locmap2, _locmap3, _locmap4);
                if (abs(_oldValue - value.valD) > 5) {
                    _oldValue = value.valD;
                    servObj.write(_oldValue);
                }
            }
        }

        IoTValue execute(String command, std::vector<IoTValue> &param) {
            if (command == "rotate") { 
                if (param.size()) {
                    value.valD = param[0].valD;
                    servObj.write(value.valD);
                    regEvent(value.valD, "IoTServo");
                }
            } 
            return {}; 
        }

        void setValue(IoTValue Value) {
            value = Value;
            if (value.isDecimal & (_oldValue != value.valD)) {
                _oldValue = value.valD;
                servObj.write(_oldValue);
                regEvent(value.valD, "IoTServo");
            }
        }

        ~IoTServo() {};
};

void* getAPI_IoTServo(String subtype, String param) {
    if (subtype == F("IoTServo")) {
        return new IoTServo(param);
    } else {
        return nullptr;
    }
}
