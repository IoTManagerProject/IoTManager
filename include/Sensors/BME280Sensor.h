#pragma once

#include <Adafruit_BME280.h>

class BME280Sensor : public BaseSensor {
   public:
    BME280Sensor(const String& name, const String& assign) : BaseSensor(name, assign, VT_FLOAT) {
        if (assign == "temperature") {
            obj = bme.getTemperatureSensor();
        } else if (assign == "humidity") {
            obj = bme.getHumiditySensor();
        } else if (assign == "pressure") {
            obj = bme.getPressureSensor();
        } else if (assign == "altitude") {
            obj = bme.getPressureSensor();
        };
        bme.begin(hexStringToUint8(getAssign()));
    }

    const String onGetValue() override {
        String assign = getAssign();
        float value;
        if (assign == "temperature") {
            value = bme.readTemperature();
        } else if (assign == "humidity") {
            value = bme.readHumidity();
        } else if (assign == "pressure") {
            value = bme.readPressure();
            value = value / 1.333224;
        } else if (assign == "altitude") {
            value = bme.readAltitude(1013.25);
        }

        return String(value, DEC);
    }

   public:
    Adafruit_BME280 bme;

   private:
    Adafruit_Sensor* obj;
};