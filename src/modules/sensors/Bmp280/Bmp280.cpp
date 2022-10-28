/******************************************************************
  Used Adafruit BMP280 Driver (Barometric Pressure Sensor)
  Support for BMP280
  https://github.com/adafruit/Adafruit_BMP280_Library
 ******************************************************************/

#include "Global.h"
#include "classes/IoTItem.h"

#include <Adafruit_BMP280.h>
#include <map>

std::map<String, Adafruit_BMP280*> bmps;

class Bmp280t : public IoTItem {
   private:
    Adafruit_BMP280* _bmp;

   public:
    Bmp280t(Adafruit_BMP280* bmp, String parameters) : IoTItem(parameters) {
        _bmp = bmp;
    }

    void doByInterval() {
        value.valD = _bmp->readTemperature();
        if (value.valD != NAN && value.valD < 150)
            regEvent(value.valD, "Bmp280t");
        else
            SerialPrint("E", "Sensor Bmp280t", "Error", _id);
    }

    ~Bmp280t(){};
};

class Bmp280p : public IoTItem {
   private:
    Adafruit_BMP280* _bmp;

   public:
    Bmp280p(Adafruit_BMP280* bmp, String parameters) : IoTItem(parameters) {
        _bmp = bmp;
    }

    void doByInterval() {
        value.valD = _bmp->readPressure();
        if (value.valD != NAN && value.valD > 0) {
            value.valD = value.valD / 1.333224 / 100;
            regEvent(value.valD, "Bmp280p");
        } else
            SerialPrint("E", "Sensor Bmp280p", "Error", _id);
    }

    ~Bmp280p(){};
};

void* getAPI_Bmp280(String subtype, String param) {
    if (subtype == F("Bmp280t") || subtype == F("Bmp280p")) {
        String addr;
        jsonRead(param, "addr", addr);
        if (addr == "") {
            scanI2C();
            return nullptr;
        }
    
       if (bmps.find(addr) == bmps.end()) {
           bmps[addr] = new Adafruit_BMP280();
           bmps[addr]->begin(hexStringToUint8(addr));
       }
    
       if (subtype == F("Bmp280t")) {
           return new Bmp280t(bmps[addr], param);
       } else if (subtype == F("Bmp280p")) {
           return new Bmp280p(bmps[addr], param);
       }
    }
    
    return nullptr;
}
