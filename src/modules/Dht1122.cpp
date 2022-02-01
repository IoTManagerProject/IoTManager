/******************************************************************
  Used DHT Temperature & Humidity Sensor library for Arduino & ESP32.
  Support for DHT11 and DHT22/AM2302/RHT03
  https://github.com/beegee-tokyo/arduino-DHTesp
 ******************************************************************/


#include "Global.h"
#include "Classes/IoTSensor.h"

#include "DHTesp.h"
#include <map>


std::map<int, DHTesp*> dhts;

class Dht1122t : public IoTSensor {
   private:
    int _pin; 
   
   public:
    Dht1122t(String parameters): IoTSensor(parameters) {
        jsonRead(parameters, "pin", _pin);
    }
    
    void doByInterval() {
        float value = dhts[_pin]->getTemperature();
        if (String(value) != "nan") regEvent(value, "Dht1122t");
            else SerialPrint("E", "Sensor DHTt", "Error");  
    }

    ~Dht1122t();
};


class Dht1122h : public IoTSensor {
   private:
    int _pin; 

   public:
    Dht1122h(String parameters): IoTSensor(parameters) {
        jsonRead(parameters, "pin", _pin);
    }
    
    void doByInterval() {
        float value = dhts[_pin]->getHumidity();
        if (String(value) != "nan") regEvent(value, "Dht1122h");
            else SerialPrint("E", "Sensor DHTh", "Error");  
    }

    ~Dht1122h();
};


void* getAPI_Dht1122(String subtype, String param) {    
    int pin;
    String senstype;
    jsonRead(param, "pin", pin);
    jsonRead(param, "senstype", senstype);

    if (dhts.find(pin) == dhts.end()) {
        DHTesp* dht = new DHTesp();
        
        if (senstype == "dht11") {
            dht->setup(pin, DHTesp::DHT11);
        } else if (senstype == "dht22") {
            dht->setup(pin, DHTesp::DHT22);
        }

        dhts[pin] = dht;
    }

    if (subtype == F("Dht1122t")) {
        return new Dht1122t(param);
    } else if (subtype == F("Dht1122h")) {
        return new Dht1122h(param);
    } else {
        return nullptr;
    }
}
