#include "Global.h"
#include "classes/IoTItem.h"

#include "Wire.h"
#include "SHT2x.h"


SHT2x* sht = nullptr;

class Sht20t : public IoTItem {
   public:
    Sht20t(String parameters): IoTItem(parameters) { }
    
    void doByInterval() {
        sht->read();
        value.valD = sht->getTemperature();
        if (value.valD > -46.85F) regEvent(value.valD, "Sht20t");  
            else SerialPrint("E", "Sensor Sht20t", "Error");  
    }

    ~Sht20t();
};

class Sht20h : public IoTItem {
   public:
    Sht20h(String parameters): IoTItem(parameters) { }
    
    void doByInterval() {
        sht->read();
        value.valD = sht->getHumidity();
        if (value.valD != -6) regEvent(value.valD, "Sht20h");  
            else SerialPrint("E", "Sensor Sht20h", "Error");
    }   

    ~Sht20h();
};


void* getAPI_Sht20(String subtype, String param) {
        if (!sht) {
           sht = new SHT2x;
           if (sht) sht->begin();
        }

        if (subtype == F("Sht20t")) {
            return new Sht20t(param);
        } else if (subtype == F("Sht20h")) {
            return new Sht20h(param);
        } else {
            return nullptr;
        }
}
