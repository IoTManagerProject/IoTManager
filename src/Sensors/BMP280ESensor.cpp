#include "Sensors/BME280Sensor.h"

#include "Global.h"
#include "Events.h"
#include "MqttClient.h"
#include "Utils/PrintMessage.h"

Adafruit_BME280 bme;
class BME280Sensor : public SensorItem {
   public:
    BME280Sensor(const String& name, const String& assign) : SensorItem(name, assign) {
        if (name == "temperature") {
            obj = bme.getTemperatureSensor();
        } else if (assign == "humidity") {
            obj = bme.getHumiditySensor();
        } else if (assign == "pressure") {
            obj = bme.getPressureSensor();
        } else if (assign == "altitude") {
            obj = bme.getPressureSensor();
        };
    }

    void onAssign() override {
        bme.begin(hexStringToUint8(getAssign()));
    }

    bool onRead() override {
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

        liveData.writeFloat(getName(), value);
        Scenario::fire(getName());
        MqttClient::publishStatus(VT_FLOAT, getName(), String(value, 2));

        return true;
    }

   private:
    Adafruit_Sensor* obj;
};