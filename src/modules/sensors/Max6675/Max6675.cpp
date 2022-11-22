/******************************************************************
  Used  MAX6675 Driver (temperature sensor)
  Support for MAX6675

   https://github.com/adafruit/MAX6675-library

  adapted for version 4 @Serghei63
 ******************************************************************/

#include "Global.h"
#include "classes/IoTItem.h"

#include "max6675.h"
#include <map>

MAX6675* thermocouple = nullptr;

class MAX6675t : public IoTItem {
   public:
    MAX6675t(String parameters) : IoTItem(parameters) {
        int thermoDO = jsonReadInt(parameters, "DO");
        int thermoCS = jsonReadInt(parameters, "CS");
        int thermoCLK = jsonReadInt(parameters, "CLK");
        thermocouple = new MAX6675(thermoCLK, thermoCS, thermoDO);
    }

    void doByInterval() {
        value.valD = thermocouple->readCelsius();
        if (String(value.valD) != "nan") {
            regEvent(value.valD, "Max6675t");
        } else {
            SerialPrint("E", "Sensor Max6675t", "Error", _id);
        }
    }

    ~MAX6675t(){};
};

void* getAPI_Max6675(String subtype, String param) {
    if (subtype == F("Max6675t")) {
        return new MAX6675t(param);
    } else {
        return nullptr;
    }
}
