#include "Utils/JsonUtils.h"
#include "Utils/SerialPrint.h"
#include "Class/ScenarioClass3.h"
#include "Class/IoTVariable.h"

IoTVariable::IoTVariable() {}
IoTVariable::~IoTVariable() {}

String IoTVariable::execute(String command) { return "";}
void IoTVariable::selfExec() {}
void IoTVariable::loop() {}
String IoTVariable::getValue(String key) { return "";}

void IoTVariable::init(String key, String id) {
    _key = key;
    _id = id;
}

void IoTVariable::regEvent(String value, String consoleInfo = "") {
    eventGen2(_id, String(value));
    jsonWriteStr(configLiveJson, _id, String(value));
    publishStatus(_id, String(value));
    SerialPrint("I", "Variable", "'" + _id + "' data: " + String(value) + "' " + consoleInfo);
}

String IoTVariable::getKey() {
    return _key;
}

String IoTVariable::getID() {
    return _id;
};

String IoTVariable::loadValue(String id) {
    return jsonReadStr(configStoreJson, id); //прочитали из памяти
};