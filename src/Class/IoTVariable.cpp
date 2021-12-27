#include "Utils/JsonUtils.h"
#include "Utils/SerialPrint.h"
#include "Class/ScenarioClass3.h"
#include "Class/IoTVariable.h"

IoTVariable::IoTVariable() {}
IoTVariable::~IoTVariable() {}

String execute(String command) {}
void IoTVariable::loop() {}

void IoTVariable::init(String key, String value) {
    
}

void setValue(String value) {

};

void getValue(String value ){

};

void IoTVariable::regEvent(String value, String consoleInfo = "") {
    eventGen2(_key, String(value));
    jsonWriteStr(configLiveJson, _key, String(value));
    publishStatus(_key, String(value));
    SerialPrint("I", "Sensor", "'" + _key + "' data: " + String(value) + "' " + consoleInfo);
}