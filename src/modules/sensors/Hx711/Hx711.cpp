#include "Global.h"
#include "classes/IoTItem.h"

#include <GyverHX711.h>


class GyverHX711g : public IoTItem {

   private:
    GyverHX711* _thermocouple = nullptr;

   public:
    GyverHX711g(String parameters) : IoTItem(parameters) {
        int data, clock, chan;
        jsonRead(parameters, "data", data);
        jsonRead(parameters, "clock", clock);
        jsonRead(parameters, "chan", chan);
        _thermocouple = new GyverHX711(data, clock, chan);
    
        _thermocouple->tare();    // калибровка нуля
    }

    void doByInterval() {
        if (!_thermocouple->available()) return;
        
        value.valD = _thermocouple->read();
        regEvent(value.valD, "Hx711");
    }

    IoTValue execute(String command, std::vector<IoTValue> &param) {
        if (command == "tare") { 
            _thermocouple->tare();
        } else if (command == "sleepMode") { 
            if (param.size() == 1) {
                _thermocouple->sleepMode(param[0].valD);
            }
        } else if (command == "read") { 
            value.valD = _thermocouple->read();
            regEvent(value.valD, "Hx711");
            return value;
        } 
        return {};  
    }

    ~GyverHX711g() {
        if (_thermocouple) delete _thermocouple;
    };
};

void* getAPI_Hx711(String subtype, String param) {
    if (subtype == F("Hx711")) {
        return new GyverHX711g(param);
    } else {
        return nullptr;
    }
}
