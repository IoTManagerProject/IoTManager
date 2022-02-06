/******************************************************************
  Used Adafruit BME280 Driver (Barometric Pressure Sensor)
  Support for BME280
  https://github.com/adafruit/Adafruit_BME280_Library
 ******************************************************************/


#include "Global.h"
#include "Classes/IoTItem.h"

#include <Adafruit_BME280.h>
#include <map>


std::map<String, Adafruit_BME280*> bmes;

class Bme280t : public IoTItem {
   private:
    Adafruit_BME280* _bme; 
   
   public:
    Bme280t(Adafruit_BME280* bme, String parameters): IoTItem(parameters) {
        _bme = bme;
    }
    
    void doByInterval() {
        float value = _bme->readTemperature();
        if (value < 145) regEvent(value, "Bme280t");
            else SerialPrint("E", "Sensor Bme280t", "Error");  
    }

    ~Bme280t();
};

  
class Bme280h : public IoTItem {
   private:
    Adafruit_BME280* _bme; 

   public:
    Bme280h(Adafruit_BME280* bme, String parameters): IoTItem(parameters) {
        _bme = bme;
    }
    
    void doByInterval() {
        float value = _bme->readHumidity();
        if (value < 100) regEvent(value, "Bme280h");
            else SerialPrint("E", "Sensor Bme280h", "Error");  
    }

    ~Bme280h();
};


class Bme280p : public IoTItem {
   private:
    Adafruit_BME280* _bme; 

   public:
    Bme280p(Adafruit_BME280* bme, String parameters): IoTItem(parameters) {
        _bme = bme;
    }
    
    void doByInterval() {
        float value = _bme->readPressure();
        if (value > 0) {
            value = value / 1.333224 / 100;
            regEvent(value, "Bme280p");
        } else SerialPrint("E", "Sensor Bme280p", "Error");  
    }

    ~Bme280p();
};


void* getAPI_Bme280(String subtype, String param) {    
    String addr;
    jsonRead(param, "addr", addr);

    if (bmes.find(addr) == bmes.end()) {
        bmes[addr] = new Adafruit_BME280();
        bmes[addr]->begin(hexStringToUint8(addr));
    }

    if (subtype == F("Bme280t")) {
        return new Bme280t(bmes[addr], param);
    } else if (subtype == F("Bme280h")) {
        return new Bme280h(bmes[addr], param);
    } else if (subtype == F("Bme280p")) {
        return new Bme280p(bmes[addr], param);
    } else {
        return nullptr;
    }
}
