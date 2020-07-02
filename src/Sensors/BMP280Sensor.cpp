#include "Sensors/BMP280Sensor.h"

#include "Global.h"
#include "Events.h"
#include "MqttClient.h"
#include "Utils/PrintMessage.h"

namespace BMP280Sensor {
Adafruit_BMP280 bmp;
Adafruit_Sensor *bmp_temp = bmp.getTemperatureSensor();
Adafruit_Sensor *bmp_pressure = bmp.getPressureSensor();

String bmp280T_value_name = "";
String bmp280P_value_name = "";

void bmp280T_reading() {
    float value = 0;
    sensors_event_t temp_event;
    bmp_temp->getEvent(&temp_event);
    value = temp_event.temperature;
    liveData.write(bmp280T_value_name, String(value));
    Events::fire(bmp280T_value_name);
    MqttClient::publishStatus(bmp280T_value_name, String(value));
    Serial.println("[I] sensor '" + bmp280T_value_name + "' data: " + String(value));
}

void bmp280P_reading() {
    float value = 0;
    sensors_event_t pressure_event;
    bmp_pressure->getEvent(&pressure_event);
    value = pressure_event.pressure;
    value = value / 1.333224;
    liveData.write(bmp280P_value_name, String(value));
    Events::fire(bmp280P_value_name);
    MqttClient::publishStatus(bmp280P_value_name, String(value));
    Serial.println("[I] sensor '" + bmp280P_value_name + "' data: " + String(value));
}
}  // namespace BMP280Sensor