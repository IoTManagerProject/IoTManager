#include "Consts.h"
#ifdef EnableSensorAnalog
#include "items/vSensorAnalog.h"
#include "Class/LineParsing.h"
#include "Global.h"
#include "BufferExecute.h"
#include <Arduino.h>

SensorAnalog::SensorAnalog(String key, unsigned long interval, unsigned int adcPin, int map1, int map2, int map3, int map4, float c) {
    _interval = interval * 1000;
    _key = key;
    _adcPin = adcPin;

    _map1 = map1;
    _map2 = map2;
    _map3 = map3;
    _map4 = map4;

    _c = c;
}

SensorAnalog::~SensorAnalog() {}

void SensorAnalog::loop() {
    currentMillis = millis();
    difference = currentMillis - prevMillis;
    if (difference >= _interval) {
        prevMillis = millis();
        readAnalog();
    }
}

void SensorAnalog::readAnalog() {
    int value;
#ifdef ESP32
    value = analogRead(_adcPin);
#endif
#ifdef ESP8266
    value = analogRead(A0);
#endif

    value = map(value, _map1, _map2, _map3, _map4);
    float valueFloat = value * _c;

    eventGen2(_key, String(valueFloat));
    jsonWriteStr(configLiveJson, _key, String(valueFloat));
    publishStatus(_key, String(valueFloat));
    SerialPrint("I", "Sensor", "'" + _key + "' data: " + String(valueFloat));
}

MySensorAnalogVector* mySensorAnalog = nullptr;

void analogAdc() {
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

    static bool firstTime = true;
    if (firstTime) mySensorAnalog = new MySensorAnalogVector();
    firstTime = false;
    mySensorAnalog->push_back(SensorAnalog(key, interval.toInt(), pin.toInt(), map1, map2, map3, map4, c.toFloat()));
}
#endif
