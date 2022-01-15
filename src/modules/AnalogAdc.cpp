#include "modules/AnalogAdc.h"

class AnalogAdc : public IoTSensor {
   public:
    // setup()
    AnalogAdc(String parameters) {
        init(jsonReadStr(parameters, "key"), jsonReadStr(parameters, "id"), jsonReadInt(parameters, "int"));

        _pin = jsonReadInt(parameters, "pin");
    }

    // loop()
    void doByInterval() {
        float value = analogRead(_pin);

        regEvent((String)value, "analog");
    }

    ~AnalogAdc();

   private:
    unsigned int _pin;
};

void* getAPI_AnalogAdc(String parameters) {
    String subtype;
    if (!jsonRead(parameters, F("subtype"), subtype)) {  //если нет такого ключа в представленном json или он не валидный
        SerialPrint(F("E"), F("Config"), F("json error AnalogAdc"));
        return nullptr;
    } else {
        if (subtype == F("AnalogAdc")) {
            return new AnalogAdc(parameters);
        } else {
            return nullptr;
        }
    }
}
