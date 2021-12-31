#include "Utils/JsonUtils.h"
#include "Utils/SerialPrint.h"
#include "Class/ScenarioClass3.h"
#include "Class/IoTVariable.h"

IoTVariable::IoTVariable() {}
IoTVariable::~IoTVariable() {}

String IoTVariable::execute(String command) { return "";}
void IoTVariable::selfExec() {}
void IoTVariable::loop() {}

void IoTVariable::init(String key, String id) {
    _key = key;
    _id = id;
}

void IoTVariable::regEvent(String value, String consoleInfo = "") {
    eventGen2(_key, String(value));
    jsonWriteStr(configLiveJson, _key, String(value));
    publishStatus(_key, String(value));
    SerialPrint("I", "Sensor", "'" + _key + "' data: " + String(value) + "' " + consoleInfo);
}

String IoTVariable::getKey() {
    return _key;
}

String IoTVariable::getID() {
    return _id;
};