#include "items/vSensorDht.h"

#include <Arduino.h>

#include "BufferExecute.h"
#include "Class/LineParsing.h"
#include "Global.h"

DHTesp* dht = nullptr;

SensorDht::SensorDht(const params& paramsTmp, const params& paramsHum) {
    _paramsTmp = params(paramsTmp);
    _paramsHum = params(paramsHum);

    if (!dht) {
        dht = new DHTesp();
    }

    dht->setup(_paramsTmp.pin, DHTesp::DHT11);

    //dht->getMinimumSamplingPeriod()
}

SensorDht::~SensorDht() {}

void SensorDht::loop() {
    //currentMillis = millis();
    //difference = currentMillis - prevMillis;
    //if (difference >= _myParams.interval) {
    //    prevMillis = millis();
    //    readTmpHum();
    //}
}

void SensorDht::readTmpHum() {
    //float tmp;
    //float hum;
    //tmp = dht->getTemperature();
    //hum = dht->getHumidity();
    //
    //if (String(tmp) != "nan" && String(hum) != "nan") {
    //    if (_myParams.type == "tmp") {
    //    }
    //
    //    if (_myParams.type == "hum") {
    //    }
    //
    //    tmp = tmp * _tmpSet.c;
    //    hum = hum * _humSet.c;
    //
    //    if (_tmpSet.interval > 0) {
    //        eventGen2(_tmpSet.key, String(tmp));
    //        jsonWriteStr(configLiveJson, _tmpSet.key, String(tmp));
    //        publishStatus(_tmpSet.key, String(tmp));
    //        SerialPrint("I", "Sensor", "'" + _tmpSet.key + "' data: " + String(tmp));
    //    }
    //
    //    if (_humSet.interval > 0) {
    //        eventGen2(_humSet.key, String(hum));
    //        jsonWriteStr(configLiveJson, _humSet.key, String(hum));
    //        publishStatus(_humSet.key, String(hum));
    //        SerialPrint("I", "Sensor", "'" + _humSet.key + "' data: " + String(hum));
    //    }
    //
    //} else {
    //    SerialPrint("E", "Sensor DHT", "Error");
    //}
}

//if (dht->getStatus() != 0 && counter < 5) {
//    counter++;
//    SerialPrint("E", "Sensor", "Disconnected " + String(counter) + " " + dht->getStatusString());
//} else {

MySensorDhtVector* mySensorDht = nullptr;

void dhtSensor() {
    myLineParsing.update();
    String type = myLineParsing.gtype();
    String value = myLineParsing.gval();
    String interval = myLineParsing.gint();
    String pin = myLineParsing.gpin();
    String key = myLineParsing.gkey();
    String c = myLineParsing.gc();
    myLineParsing.clear();
    static params paramsTmp;
    static params paramsHum;
    if (value = "tmp") {
        paramsTmp.type = type;
        paramsTmp.value = value;
        paramsTmp.key = key;
        paramsTmp.interval = interval.toInt() * 1000;
        paramsTmp.pin = pin.toInt();
        paramsTmp.c = c.toFloat();
    }
    if (value = "hum") {
        paramsHum.type = type;
        paramsHum.value = value;
        paramsHum.key = key;
        paramsHum.interval = interval.toInt() * 1000;
        paramsHum.pin = pin.toInt();
        paramsHum.c = c.toFloat();
    }
    dht_EnterCounter++;
    if (dht_EnterCounter == 2) {
        static bool firstTime = true;
        if (firstTime) mySensorDht = new MySensorDhtVector();
        firstTime = false;
        mySensorDht->push_back(SensorDht(paramsTmp, paramsHum));
    }
}
