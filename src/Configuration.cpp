#include "Configuration.h"
#include "modules/IoTSensorA.h"

std::vector<IoTSensor*> iotSensors;

void configure(String path) {
    File file = seekFile(path);
    file.find("[");
    while (file.available()) {
        String jsonArrayElement = file.readStringUntil('}') + "}";
        if (jsonArrayElement.startsWith(",")) {
            jsonArrayElement = jsonArrayElement.substring(1, jsonArrayElement.length());  //это нужно оптимизировать в последствии
        }
        String subtype;
        if (jsonRead(jsonArrayElement, F("subtype"), subtype)) {
            if (subtype == F("button-out")) {
                //=============================
            } else if (subtype == F("pwm-out")) {
                //=============================
            } else if (subtype == F("analog-adc")) {
                mySensorAnalog = new IoTSensorA(jsonArrayElement);
                iotSensors.push_back(mySensorAnalog);
            } else {
                SerialPrint(F("E"), F("Config"), "type not exist " + subtype);
            }
        } else {
            SerialPrint(F("E"), F("Config"), F("json error"));
        }
    }
    file.close();
}