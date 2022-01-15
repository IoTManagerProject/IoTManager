#include "modules/IoTSensorA.h"

IoTSensorA::IoTSensorA(String parameters) {
    init(jsonReadStr(parameters, "key"), jsonReadStr(parameters, "id"), jsonReadInt(parameters, "int"));

    _pin = jsonReadInt(parameters, "pin");
}

IoTSensorA::~IoTSensorA() {}

void IoTSensorA::doByInterval() {
    float value = analogRead(_pin);

    regEvent((String)value, "analog");  //обязательный вызов для отправки результата работы
}

IoTSensorA* mySensorAnalog;
