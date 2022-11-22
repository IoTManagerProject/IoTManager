/******************************************************************
  Used DHT Temperature & Humidity Sensor library for Arduino & ESP32.
  Support for DHT11 and DHT22/AM2302/RHT03
  https://github.com/beegee-tokyo/arduino-DHTesp
 ******************************************************************/

#include "Global.h"
#include "classes/IoTItem.h"

#include "DHTesp.h"
#include <map>

std::map<int, DHTesp*> dhts;

class Dht1122t : public IoTItem {
   private:
    DHTesp* _dht;

   public:
    Dht1122t(DHTesp* dht, String parameters) : IoTItem(parameters) {
        _dht = dht;
    }

    void doByInterval() {
        value.valD = _dht->getTemperature();
        if (String(value.valD) != "nan")
            regEvent(value.valD, "Dht1122t");
        else
            SerialPrint("E", "Sensor DHTt", "Error", _id);
    }

    ~Dht1122t(){};
};

class Dht1122h : public IoTItem {
   private:
    DHTesp* _dht;

   public:
    Dht1122h(DHTesp* dht, String parameters) : IoTItem(parameters) {
        _dht = dht;
    }

    void doByInterval() {
        value.valD = _dht->getHumidity();
        if (String(value.valD) != "nan")
            regEvent(value.valD, "Dht1122h");
        else
            SerialPrint("E", "Sensor DHTh", "Error", _id);
    }

    ~Dht1122h(){};
};

void* getAPI_Dht1122(String subtype, String param) {
    if (subtype == F("Dht1122t") || subtype == F("Dht1122h")) {
        int pin;
        String senstype;
        jsonRead(param, "pin", pin);
        jsonRead(param, "senstype", senstype);
    
       if (dhts.find(pin) == dhts.end()) {
           dhts[pin] = new DHTesp();
    
           if (senstype == "dht11") {
               dhts[pin]->setup(pin, DHTesp::DHT11);
           } else if (senstype == "dht22") {
               dhts[pin]->setup(pin, DHTesp::DHT22);
           }
       }
    
       if (subtype == F("Dht1122t")) {
           return new Dht1122t(dhts[pin], param);
       } else if (subtype == F("Dht1122h")) {
           return new Dht1122h(dhts[pin], param);
       }
    }
    
    return nullptr;
}
