/******************************************************************
  Used Adafruit BMP280 Driver (Barometric Pressure Sensor)
  Support for BMP280
  https://github.com/adafruit/Adafruit_BMP280_Library
 ******************************************************************/


#include "Global.h"
#include "Classes/IoTItem.h"

#include <Adafruit_BMP280.h>
#include <map>


std::map<String, Adafruit_BMP280*> bmps;

class Bmp280t : public IoTItem {
   private:
    Adafruit_BMP280* _bmp; 
   
   public:
    Bmp280t(Adafruit_BMP280* bmp, String parameters): IoTItem(parameters) {
        _bmp = bmp;
    }
    
    void doByInterval() {
        float value = _bmp->readTemperature();
        if (String(value) != "nan") regEvent(value, "Bmp280t");
            else SerialPrint("E", "Sensor DHTt", "Error");  
    }

    ~Bmp280t();
};

  
class Bmp280p : public IoTItem {
   private:
    Adafruit_BMP280* _bmp; 

   public:
    Bmp280p(Adafruit_BMP280* bmp, String parameters): IoTItem(parameters) {
        _bmp = bmp;
    }
    
    void doByInterval() {
        float value = _bmp->readPressure();
        if (String(value) != "nan") {
            value = value / 1.333224 / 100;
            regEvent(value, "Bmp280p");
        } else SerialPrint("E", "Sensor DHTh", "Error");  
    }

    ~Bmp280p();
};


void* getAPI_Bmp280(String subtype, String param) {    
    String addr;
    jsonRead(param, "addr", addr);

    if (bmps.find(addr) == bmps.end()) {
        bmps[addr] = new Adafruit_BMP280();
        bmps[addr]->begin(hexStringToUint8(addr));
    }

    if (subtype == F("Bmp280t")) {
        return new Bmp280t(bmps[addr], param);
    } else if (subtype == F("Bmp280p")) {
        return new Bmp280p(bmps[addr], param);
    } else {
        return nullptr;
    }
}
