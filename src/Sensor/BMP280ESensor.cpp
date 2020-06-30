#include <Sensor/BMP280ESensor.h>

Adafruit_BME280 bme;
Adafruit_Sensor *bme_temp = bme.getTemperatureSensor();
Adafruit_Sensor *bme_pressure = bme.getPressureSensor();
Adafruit_Sensor *bme_humidity = bme.getHumiditySensor();

String bme280T_value_name;
String bme280P_value_name;
String bme280H_value_name;
String bme280A_value_name;