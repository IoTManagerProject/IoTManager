/******************************************************************
  Used ClosedCube HDC1080 Driver (temperature and humidity sensor)
  Support for HDC1080
  https://github.com/closedcube/ClosedCube_HDC1080_Arduino

  adapted for version 4 @Serghei63
 ******************************************************************/
#include "Global.h"
#include "classes/IoTItem.h"

#include "Wire.h"
#include "ClosedCube_HDC1080.h"
#include <map>

// to do убрать глобальный экземпляр
ClosedCube_HDC1080 hdc1080;

class Hdc1080t : public IoTItem {
   public:
    Hdc1080t(String parameters) : IoTItem(parameters) {}

    void doByInterval() {
        value.valD = hdc1080.readTemperature();
        if (value.valD < 124)
            regEvent(value.valD, "Hdc1080t");
        else
            SerialPrint("E", "Sensor Hdc1080t", "Error", _id);
    }

    ~Hdc1080t(){};
};

class Hdc1080h : public IoTItem {
   public:
    Hdc1080h(String parameters) : IoTItem(parameters) {}

    void doByInterval() {
        value.valD = hdc1080.readHumidity();
        if (value.valD < 99)
            regEvent(value.valD, "Hdc1080h");
        else
            SerialPrint("E", "Sensor Hdc1080h", "Error", _id);
    }

    ~Hdc1080h(){};
};

void* getAPI_Hdc1080(String subtype, String param) {
    if (subtype == F("Hdc1080t")) {
        hdc1080.begin(0x40);
        return new Hdc1080t(param);
    } else if (subtype == F("Hdc1080h")) {
        hdc1080.begin(0x40);
        return new Hdc1080h(param);
    } else {
        return nullptr;
    }
}