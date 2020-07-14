#include "Sensors/BME280Sensor.h"

#include "Global.h"
#include "MqttClient.h"
#include "PrintMessage.h"

Adafruit_BME280 bme;
class BME280Sensor : public BaseSensor {
   public:
    BME280Sensor(const String& name) : BaseSensor(name, VT_FLOAT) {
    }

    void onAssign() override {
        String assign = getAssign();
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

   private:
    Adafruit_Sensor* obj;
};