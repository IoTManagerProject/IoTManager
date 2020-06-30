#include <Sensor/BMP280PSensor.h>

Adafruit_BMP280 bmp;
Adafruit_Sensor *bmp_temp = bmp.getTemperatureSensor();
Adafruit_Sensor *bmp_pressure = bmp.getPressureSensor();

String bmp280T_value_name = "";
String bmp280P_value_name = "";
