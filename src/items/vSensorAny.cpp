#include "Consts.h"
#ifdef EnableSensorAny
#include <Arduino.h>

#include "BufferExecute.h"
#include "Class/LineParsing.h"
#include "Global.h"
#include "YourSensor.h"
#include "items/vSensorAny.h"

SensorAny::SensorAny(const String& paramsAny) {
    _paramsAny = paramsAny;

    _interval = jsonReadInt(_paramsAny, "int");
    _c = jsonReadFloat(_paramsAny, "c");
    _k = jsonReadFloat(_paramsAny, "k");
    _key = jsonReadStr(_paramsAny, "key");
    _addr = jsonReadStr(_paramsAny, "addr");
    _type = jsonReadStr(_paramsAny, "type");
    _val = jsonReadStr(_paramsAny, "val");
    _descr = jsonReadStr(_paramsAny, "descr");
}

SensorAny::~SensorAny() {}

void SensorAny::loop() {
    difference = millis() - prevMillis;
    if (difference >= _interval * 1000) {
        prevMillis = millis();
        read();
    }
}

void SensorAny::read() {
    float value = yourSensorReading(_type, _paramsAny);

    value = value * _c;

    eventGen2(_key, String(value));
    jsonWriteStr(configLiveJson, _key, String(value));
    publishStatus(_key, String(value));
    String path = mqttRootDevice + "/" + _key + "/status";
    String json = "{}";
    jsonWriteStr(json, "status", String(value));
    String MyJson = json;
    jsonWriteStr(MyJson, "topic", path);
    ws.textAll(MyJson);
    SerialPrint("I", "Sensor", "'" + _key + "' data: " + String(value));
}

MySensorAnyVector* mySensorAny = nullptr;

void AnySensorExecute() {
    String key = sCmd.order();
    String command = sCmd.next();
    
    if (command == "cmd1") {
        SerialPrint("I", "Sensor", key + " выполняет cmd1 без параметра");
    } 
    else if (command == "cmd2") {
        String par = sCmd.next();
        SerialPrint("I", "Sensor", key + " выполняет cmd2 c параметром " + par);
    }
    else if (command == "cmd3") {
        String par = sCmd.next();
        SerialPrint("I", "Sensor", key + " выполняет cmd3 c параметром " + par);
    }
}

void AnySensor() {
    String params = "{}";
    myLineParsing.update();
    String key = myLineParsing.gkey();
    jsonWriteStr(params, "key", key);
    jsonWriteStr(params, "addr", myLineParsing.gaddr());
    jsonWriteStr(params, "int", myLineParsing.gint());
    jsonWriteStr(params, "c", myLineParsing.gc());
    jsonWriteStr(params, "k", myLineParsing.gk());
    jsonWriteStr(params, "val", myLineParsing.gval());
    jsonWriteStr(params, "type", myLineParsing.gtype());
    jsonWriteStr(params, "descr", myLineParsing.gdescr());
    myLineParsing.clear();

    static bool firstTime = true;
    if (firstTime) {
        mySensorAny = new MySensorAnyVector();
        sCmd.addCommand(key.c_str(), AnySensorExecute);
    }
    firstTime = false;
    mySensorAny->push_back(SensorAny(params));
}
#endif