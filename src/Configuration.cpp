#include "Configuration.h"

void configure(String& path) {
    File file = seekFile(path);
    while (file.available()) {
        String jsonArrayElement = file.readStringUntil('}') + "}";
        String subtype;
        if (jsonRead(jsonArrayElement, F("subtype"), subtype)) {
            if (subtype == F("button-out")) {
                //=============================
            } else if (subtype == F("pwm-out")) {
                //=============================
            } else if (subtype == F("analog-adc")) {
                //iotSensors.push_back(IoTSensorAnalog(jsonArrayElement));
            } else {
                SerialPrint(F("E"), F("Config"), F("config.json error, type not exist"));
            }
        } else {
            SerialPrint(F("E"), F("Config"), F("config.json error, type wrong or missing"));
        }
    }
    file.close();
}