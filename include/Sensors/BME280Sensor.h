#include <Adafruit_BME280.h>

namespace BME280Sensor {
extern Adafruit_BME280 bme;
extern Adafruit_Sensor *bme_temp;
extern Adafruit_Sensor *bme_pressure;
extern Adafruit_Sensor *bme_humidity;

extern String bme280T_value_name;
extern String bme280P_value_name;
extern String bme280H_value_name;
extern String bme280A_value_name;

void bme280A_reading();
void bme280T_reading();
void bme280P_reading();
void bme280H_reading();

}