#include "Configuration.h"

extern void* getAPI_AnalogAdc(String params);

std::vector<IoTSensor*> iotSensors;

void configure(String path) {
    File file = seekFile(path);
    file.find("[");
    while (file.available()) {
        String jsonArrayElement = file.readStringUntil('}') + "}";
        if (jsonArrayElement.startsWith(",")) {
            jsonArrayElement = jsonArrayElement.substring(1, jsonArrayElement.length());  //это нужно оптимизировать в последствии
        }

        myIoTSensor = (IoTSensor*)getAPI_AnalogAdc(jsonArrayElement);
        if (myIoTSensor) {
            iotSensors.push_back(myIoTSensor);
        }
    }
    file.close();
}