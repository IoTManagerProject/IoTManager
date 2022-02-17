/******************************************************************
  Used GY21 Driver (temperature and humidity sensor)
  Support for HTY21 , SHT21 , Si7021
  https://github.com/adafruit/Adafruit_AHTX0

  adapted for version 4 Alecs Alecs
 ******************************************************************/
#include "Global.h"
#include "Classes/IoTItem.h"

#include "Wire.h"
#include "GY21.h"

GY21* sensor = nullptr;

class GY21t : public IoTItem {
   public:
    GY21t(String parameters): IoTItem(parameters) { }
    
    void doByInterval() {
        //wire->read();
        value.valD = sensor->GY21_Temperature();
        if (value.valD > -46.85F) regEvent(value.valD, "GY21");  
            else SerialPrint("E", "Sensor GY21t", "Error");  
    }

    ~GY21t();
};

class GY21h : public IoTItem {
   public:
    GY21h(String parameters): IoTItem(parameters) { }
    
    void doByInterval() {
        //sht->read();
        value.valD = sensor->GY21_Humidity();
        if (value.valD != -6) regEvent(value.valD, "GY21h");  
            else SerialPrint("E", "Sensor GY21h", "Error");
    }   

    ~GY21h();
};

void* getAPI_GY21(String subtype, String param) {
        if (!sensor) {
           sensor = new GY21;
           if (sensor) Wire.begin(SDA, SCL);
        }

        if (subtype == F("GY21t")) {
            return new GY21t(param);
        } else if (subtype == F("GY21h")) {
            return new GY21h(param);
        } else {
            return nullptr;
        }
}