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
        regEvent((String)value, "Sht20t");  
    }

    ~Sht20t();
};

class Sht20h : public IoTSensor {
   public:
    Sht20h(String parameters): IoTSensor(parameters) { }
    
    void doByInterval() {
        float value = sht->getHumidity();
        regEvent((String)value, "Sht20h");  
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
