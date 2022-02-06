#include "ESPConfiguration.h"

std::vector<IoTItem*> IoTItems;
void* getAPI(String subtype, String params);

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
            myIoTItem = (IoTItem*)getAPI(subtype, jsonArrayElement);
            if (myIoTItem) {
                IoTItems.push_back(myIoTItem);
            }
        }
    }
    file.close();
}