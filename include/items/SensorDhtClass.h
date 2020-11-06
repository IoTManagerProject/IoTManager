#pragma once
#include <Arduino.h>

#include "Class/LineParsing.h"
#include "Global.h"
#include "items/SensorConvertingClass.h"

DHTesp dht;
class SensorDhtClass : public SensorConvertingClass {
   public:
    SensorDhtClass() : SensorConvertingClass(){};

    void SensorDhtInit() {
        if (_type == "dht11") {
            dht.setup(_pin.toInt(), DHTesp::DHT11);
        }
        if (_type == "dht22") {
            dht.setup(_pin.toInt(), DHTesp::DHT22);
        }
        sensorReadingMap10sec += _key + ",";

        //to do если надо будет читать несколько dht
        //dhtEnterCounter++;
        //jsonWriteInt(configOptionJson, _key + "_num", dhtEnterCounter);

        jsonWriteStr(configOptionJson, _key + "_map", _map);
        jsonWriteStr(configOptionJson, _key + "_с", _c);
    }

    void SensorDhtReadTemp(String key) {
        //to do если надо будет читать несколько dht
        //int cnt = jsonReadInt(configOptionJson, key + "_num");
        float value;
        static int counter;
        if (dht.getStatus() != 0 && counter < 5) {
            counter++;
            //return;
        } else {
            counter = 0;
            value = dht.getTemperature();
            if (String(value) != "nan") {
                //value = this->mapping(key, value);
                float valueFl = this->correction(key, value);
                eventGen2(key, String(valueFl));
                jsonWriteStr(configLiveJson, key, String(valueFl));
                 publishStatus(key, String(valueFl));
                SerialPrint("I", "Sensor", "'" + key + "' data: " + String(valueFl));
            } else {
                Serial.println("[E] sensor '" + key);
            }
        }
    }

    void SensorDhtReadHum(String key) {
        //to do если надо будет читать несколько dht
        //int cnt = jsonReadInt(configOptionJson, key + "_num");
        float value;
        static int counter;
        if (dht.getStatus() != 0 && counter < 5) {
            counter++;
            //return;
        } else {
            counter = 0;
            value = dht.getHumidity();
            if (String(value) != "nan") {
                //value = this->mapping(key, value);
                float valueFl = this->correction(key, value);
                eventGen2(key, String(valueFl));
                jsonWriteStr(configLiveJson, key, String(valueFl));
                 publishStatus(key, String(valueFl));
                SerialPrint("I", "Sensor", "'" + key + "' data: " + String(valueFl));
            } else {
                Serial.println("[E] sensor '" + key);
            }
        }
    }
};
extern SensorDhtClass mySensorDht;