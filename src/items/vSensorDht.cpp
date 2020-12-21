#include "items/vSensorDht.h"

#include <Arduino.h>

#include "BufferExecute.h"
#include "Class/LineParsing.h"
#include "Global.h"

DHTesp* dht = nullptr;

SensorDht::SensorDht() {}

SensorDht::~SensorDht() {}

void SensorDht::tmpInit(const tmpParams& tmpSet) {
    _tmpSet = tmpParams(tmpSet);
    if (!dht) {
        dht = new DHTesp();
    }
    dht->setup(_tmpSet.pin, DHTesp::DHT11);
    _tmpSet.interval = dht->getMinimumSamplingPeriod() + _tmpSet.interval;
}

void SensorDht::humInit(const humParams& humSet) {
    _humSet = humParams(humSet);
    if (!dht) {
        dht = new DHTesp();
    }
    dht->setup(_tmpSet.pin, DHTesp::DHT11);
    _tmpSet.interval = dht->getMinimumSamplingPeriod() + _tmpSet.interval;
}

void SensorDht::loopTmp() {
    _tmpSet.currentMillis = millis();
    _tmpSet.difference = _tmpSet.currentMillis - _tmpSet.prevMillis;
    if (_tmpSet.difference >= _tmpSet.interval) {
        _tmpSet.prevMillis = millis();
        readTmp();
    }
}

void SensorDht::loopHum() {
    _humSet.currentMillis = millis();
    _humSet.difference = _humSet.currentMillis - _humSet.prevMillis;
    if (_humSet.difference >= _humSet.interval) {
        _humSet.prevMillis = millis();
        readHum();
    }
}

void SensorDht::readTmp() {
    float value;
    static int counter;
    //if (dht->getStatus() != 0 && counter < 5) {
    //    counter++;
    //    SerialPrint("E", "Sensor", "Disconnected " + String(counter) + " " + dht->getStatusString());
    //} else {
        counter = 0;
        value = dht->getTemperature();
        if (String(value) != "nan") {
            //value = map(value, _tmpSet.map1, _tmpSet.map2, _tmpSet.map3, _tmpSet.map4);
            value = value * _tmpSet.c;
            eventGen2(_tmpSet.key, String(value));
            jsonWriteStr(configLiveJson, _tmpSet.key, String(value));
            publishStatus(_tmpSet.key, String(value));
            SerialPrint("I", "Sensor", "'" + _tmpSet.key + "' data: " + String(value));
        } else {
            SerialPrint("E", "Sensor", "'" + _tmpSet.key + "' data: " + String(value));
        }
    //}
}

void SensorDht::readHum() {
    float value;
    static int counter;
    //if (dht->getStatus() != 0 && counter < 5) {
    //    counter++;
    //    SerialPrint("E", "Sensor", "Disconnected " + String(counter) + " " + dht->getStatusString());
    //} else {
        counter = 0;
        value = dht->getHumidity();
        if (String(value) != "nan") {
            //value = map(value, _humSet.map1, _humSet.map2, _humSet.map3, _humSet.map4);
            value = value * _humSet.c;
            eventGen2(_humSet.key, String(value));
            jsonWriteStr(configLiveJson, _humSet.key, String(value));
            publishStatus(_humSet.key, String(value));
            SerialPrint("I", "Sensor", "'" + _humSet.key + "' data: " + String(value));
        } else {
            SerialPrint("E", "Sensor", "'" + _humSet.key + "' data: " + String(value));
        }
    //}
}

MySensorDhtVector* mySensorDht = nullptr;

void dhtTmp() {
    myLineParsing.update();
    String interval = myLineParsing.gint();
    String pin = myLineParsing.gpin();
    String key = myLineParsing.gkey();
    String map = myLineParsing.gmap();
    String c = myLineParsing.gc();
    myLineParsing.clear();

    int map1 = selectFromMarkerToMarker(map, ",", 0).toInt();
    int map2 = selectFromMarkerToMarker(map, ",", 1).toInt();
    int map3 = selectFromMarkerToMarker(map, ",", 2).toInt();
    int map4 = selectFromMarkerToMarker(map, ",", 3).toInt();

    tmpParams buf;

    buf.interval = interval.toInt() * 1000;
    buf.key = key;
    buf.pin = pin.toInt();
    buf.map1 = map1;
    buf.map2 = map2;
    buf.map3 = map3;
    buf.map4 = map4;
    buf.c = c.toFloat();

    dhtTmp_EnterCounter++;

    static bool firstTime = true;
    if (firstTime) mySensorDht = new MySensorDhtVector();
    firstTime = false;
    mySensorDht->push_back(SensorDht());
    mySensorDht->at(dhtTmp_EnterCounter).tmpInit(buf);
}

void dhtHum() {
    myLineParsing.update();
    String interval = myLineParsing.gint();
    String pin = myLineParsing.gpin();
    String key = myLineParsing.gkey();
    String map = myLineParsing.gmap();
    String c = myLineParsing.gc();
    myLineParsing.clear();

    int map1 = selectFromMarkerToMarker(map, ",", 0).toInt();
    int map2 = selectFromMarkerToMarker(map, ",", 1).toInt();
    int map3 = selectFromMarkerToMarker(map, ",", 2).toInt();
    int map4 = selectFromMarkerToMarker(map, ",", 3).toInt();

    humParams buf;

    buf.interval = interval.toInt() * 1000;
    buf.key = key;
    buf.pin = pin.toInt();
    buf.map1 = map1;
    buf.map2 = map2;
    buf.map3 = map3;
    buf.map4 = map4;
    buf.c = c.toFloat();

    dhtHum_EnterCounter++;

    static bool firstTime = true;
    if (firstTime) mySensorDht = new MySensorDhtVector();
    firstTime = false;
    mySensorDht->push_back(SensorDht());
    mySensorDht->at(dhtHum_EnterCounter).humInit(buf);
}
