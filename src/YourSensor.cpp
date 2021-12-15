#include "YourSensor.h"

#include "Utils/StringUtils.h"

//подключаем необходимые файлы библиотеки
#include <Adafruit_AHTX0.h>

#include "ClosedCube_HDC1080.h"

//создаем объект HDC1080
ClosedCube_HDC1080 hdc1080;

//создаем объект AHTX0
Adafruit_AHTX0 aht;
Adafruit_Sensor *aht_humidity, *aht_temp;
sensors_event_t tmpEvent_t;

float yourSensorReading(String type, String addr) {
    float value;
    //========================================================HDC1080================================================================
    if (type == "HDC1080_temp") {
        HDC1080_init(addr);
        value = hdc1080.readTemperature();
    } else if (type == "HDC1080_hum") {
        HDC1080_init(addr);
        value = hdc1080.readHumidity();
    }
    //==========================================================AHTX0=================================================================
    if (type == "AHTX0_temp") {
        AHTX0_init();
        aht_temp->getEvent(&tmpEvent_t);
        value = tmpEvent_t.temperature;
    } else if (type == "typeAHTX0_hum") {
        AHTX0_init();
        aht_humidity->getEvent(&tmpEvent_t);
        value = tmpEvent_t.relative_humidity;
    }
    return value;
}

void HDC1080_init(String &addr) {
    static bool HDC1080_flag = true;
    if (HDC1080_flag) {
        hdc1080.begin(hexStringToUint8(addr));
        HDC1080_flag = false;
    }
}

void AHTX0_init() {
    static bool AHTX0_flag = true;
    if (AHTX0_flag) {
        if (!aht.begin()) {
            Serial.println("Failed to find AHT10/AHT20 chip");
            // return -127;
        }
        aht_temp = aht.getTemperatureSensor();
        aht_temp->printSensorDetails();

        aht_humidity = aht.getHumiditySensor();
        aht_humidity->printSensorDetails();
        AHTX0_flag = false;
    }
}
