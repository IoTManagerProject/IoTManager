#include "ESPConfiguration.h"

std::vector<IoTSensor*> iotSensors;
void* getAPI(String subtype, String params);

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

        String subtype;
        if (!jsonRead(jsonArrayElement, F("subtype"), subtype)) {  //если нет такого ключа в представленном json или он не валидный
            SerialPrint(F("E"), F("Config"), "json error " + subtype);
            continue;
        } else {
            myIoTSensor = (IoTSensor*)getAPI(subtype, jsonArrayElement);
            if (myIoTSensor) {
                iotSensors.push_back(myIoTSensor);
                // createWidget(jsonArrayElement);
            }
        }
    }
    file.close();
}