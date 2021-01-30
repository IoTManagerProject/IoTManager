#include "Consts.h"
#ifdef EnableSensorUltrasonic
#include "items/vSensorUltrasonic.h"

#include <Arduino.h>

#include "BufferExecute.h"
#include "Class/LineParsing.h"
#include "Global.h"

GMedian<5, int> testFilter;

SensorUltrasonic::SensorUltrasonic(String key, unsigned long interval, unsigned int trig, unsigned int echo, int map1, int map2, int map3, int map4, float c) {
    _interval = interval * 1000;
    _key = key;
    _trig = trig;
    _echo = echo;

    _map1 = map1;
    _map2 = map2;
    _map3 = map3;
    _map4 = map4;

    _c = c;

    pinMode(_trig, OUTPUT);
    pinMode(_echo, INPUT);
}

SensorUltrasonic::~SensorUltrasonic() {}

void SensorUltrasonic::loop() {
    currentMillis = millis();
    difference = currentMillis - prevMillis;
    if (difference >= _interval) {
        prevMillis = millis();
        readUltrasonic();
    }
}

void SensorUltrasonic::readUltrasonic() {
    static unsigned int counter;
    counter++;

    int value;

    digitalWrite(_trig, LOW);
    delayMicroseconds(2);
    digitalWrite(_trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(_trig, LOW);
    long duration_ = pulseIn(_echo, HIGH, 30000);  // 3000 µs = 50cm // 30000 µs = 5 m
    value = duration_ / 29 / 2;

    value = testFilter.filtered(value);

    value = map(value, _map1, _map2, _map3, _map4);
    float valueFloat = value * _c;

    if (counter > 10) {
        eventGen2(_key, String(valueFloat));
        jsonWriteStr(configLiveJson, _key, String(valueFloat));
        publishStatus(_key, String(valueFloat));
        SerialPrint("I", "Sensor", "'" + _key + "' data: " + String(valueFloat));
    }
}

MySensorUltrasonicVector* mySensorUltrasonic = nullptr;

void ultrasonic() {
    myLineParsing.update();
    String interval = myLineParsing.gint();
    String pin = myLineParsing.gpin();
    String key = myLineParsing.gkey();
    String map = myLineParsing.gmap();
    String c = myLineParsing.gc();
    myLineParsing.clear();

    unsigned int trig = selectFromMarkerToMarker(pin, ",", 0).toInt();
    unsigned int echo = selectFromMarkerToMarker(pin, ",", 1).toInt();

    int map1 = selectFromMarkerToMarker(map, ",", 0).toInt();
    int map2 = selectFromMarkerToMarker(map, ",", 1).toInt();
    int map3 = selectFromMarkerToMarker(map, ",", 2).toInt();
    int map4 = selectFromMarkerToMarker(map, ",", 3).toInt();

    static bool firstTime = true;
    if (firstTime) mySensorUltrasonic = new MySensorUltrasonicVector();
    firstTime = false;
    mySensorUltrasonic->push_back(SensorUltrasonic(key, interval.toInt(), trig, echo, map1, map2, map3, map4, c.toFloat()));
}
#endif
