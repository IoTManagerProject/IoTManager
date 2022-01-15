#include "ESPConfiguration.h"

std::vector<IoTSensor*> iotSensors;

//============================================================================================
//здесь скопируйте строку и вставьте ниже, заменив имя AnalogAdc на название вашего сенсора
extern void* getAPI_AnalogAdc(String params);
//============================================================================================

void configure(String path) {
    File file = seekFile(path);
    file.find("[");
    while (file.available()) {
        String jsonArrayElement = file.readStringUntil('}') + "}";
        if (jsonArrayElement.startsWith(",")) {
            jsonArrayElement = jsonArrayElement.substring(1, jsonArrayElement.length());  //это нужно оптимизировать в последствии
        }
        //===============================================================================================================
        //здесь нужно скопировать блок еще раз и вставить его ниже, переименовав AnalogAdc на название вашего сенсора
        myIoTSensor = (IoTSensor*)getAPI_AnalogAdc(jsonArrayElement);
        if (myIoTSensor) {
            iotSensors.push_back(myIoTSensor);
        }
        //================================================================================================================
    }
    file.close();
}