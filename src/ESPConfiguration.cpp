#include "ESPConfiguration.h"
#include "classes/IoTGpio.h"
#include "classes/IoTRTC.h"

extern IoTGpio IoTgpio;
extern IoTRTC *watch;

std::list<IoTItem*> IoTItems;
void* getAPI(String subtype, String params);

void configure(String path) {
    File file = seekFile(path);
    file.find("[");
    while (file.available()) {
        String jsonArrayElement = file.readStringUntil('}') + "}";
        if (jsonArrayElement.startsWith(",")) {
            jsonArrayElement = jsonArrayElement.substring(1, jsonArrayElement.length());  //это нужно оптимизировать в последствии
        }
        if (jsonArrayElement == "]}") {
            jsonArrayElement = "";
        }
        if (jsonArrayElement != "") {
            String subtype;
            if (!jsonRead(jsonArrayElement, F("subtype"), subtype)) {  //если нет такого ключа в представленном json или он не валидный
                SerialPrint(F("E"), F("Config"), "json error " + subtype);
                continue;
            } else {
                myIoTItem = (IoTItem*)getAPI(subtype, jsonArrayElement);
                if (myIoTItem) {
                    // пробуем спросить драйвер GPIO
                    IoTGpio* gpiotmp = myIoTItem->getGpioDriver();
                    if (gpiotmp) IoTgpio.regDriver(gpiotmp);
                    
                    // пробуем спросить драйвер RTC
                    iarduino_RTC_BASE* rtctmp = myIoTItem->getRtcDriver();
                    if (rtctmp) {
                        delete watch->objClass;
                        watch->objClass = rtctmp;
                        int valPeriod_save = watch->valPeriod;
                        watch->valPeriod = 0;
                        watch->gettime();
                        watch->valPeriod = valPeriod_save;
                    }
                    
                    IoTItems.push_back(myIoTItem);
                }
            }
        }
    }
    file.close();
    SerialPrint("i", "Config", "Configured");
}

void clearConfigure() {
    Serial.printf("Start clearing config\n");
    for (std::list<IoTItem*>::iterator it=IoTItems.begin(); it != IoTItems.end(); ++it) {
        if (*it) delete *it;
    }
    IoTItems.clear();
}