/******************************************************************
  Used Adafruit AHT20 Driver (temperature and humidity sensor)
  Support for AHT20
  https://github.com/adafruit/Adafruit_AHTX0

  adapted for version 4 @Serghei63
 ******************************************************************/


#include "Global.h"
#include "classes/IoTItem.h"

#include "Adafruit_AHTX0.h"
#include <map>

Adafruit_AHTX0 aht;
sensors_event_t temp, humidity;

class Aht20t : public IoTItem {
   public:
    Aht20t(String parameters): IoTItem(parameters) { }
    
    void doByInterval() {
        value.valD = temp.temperature;
        if (value.valD != -200) regEvent(value.valD, "Aht20t");  // TODO: найти способ понимания ошибки получения данных
            else SerialPrint("E", "Sensor AHTt", "Error");   
    }

    ~Aht20t() {};
};


class Aht20h : public IoTItem {
   public:
    Aht20h(String parameters): IoTItem(parameters) { }
    
    void doByInterval() {
        value.valD = humidity.relative_humidity;
        if (value.valD != -200) regEvent(value.valD, "Aht20h");  // TODO: найти способ понимания ошибки получения данных
            else SerialPrint("E", "Sensor AHTt", "Error");    
    }

    ~Aht20h() {};
};


void* getAPI_Aht20(String subtype, String param) {
    if (subtype == F("Aht20t")) {
        aht.begin();
        aht.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
        return new Aht20t(param);
    } else if (subtype == F("Aht20h")) {
        aht.begin();
        aht.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
        return new Aht20h(param);
    } else {
        return nullptr;
    }
}

