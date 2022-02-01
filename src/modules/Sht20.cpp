#include "Global.h"
#include "Classes/IoTSensor.h"

#include "Wire.h"
#include "SHT2x.h"


SHT2x* sht = nullptr;

class Sht20t : public IoTSensor {
   public:
    Sht20t(String parameters): IoTSensor(parameters) { }
    
    void doByInterval() {
        sht->read();
        float value = sht->getTemperature();
        if (value != -46.85) regEvent(value, "Sht20t");  
            else SerialPrint("E", "Sensor Sht20t", "Error");  
    }

    ~Sht20t();
};

class Sht20h : public IoTSensor {
   public:
    Sht20h(String parameters): IoTSensor(parameters) { }
    
    void doByInterval() {
        sht->read();
        float value = sht->getHumidity();
        if (value != -6) regEvent(value, "Sht20h");  
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
