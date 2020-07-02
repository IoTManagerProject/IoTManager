#include <Adafruit_BMP280.h>

namespace BMP280Sensor {

extern Adafruit_BMP280 bmp;
extern Adafruit_Sensor *bmp_temp;
extern Adafruit_Sensor *bmp_pressure;

extern String bmp280T_value_name;
extern String bmp280P_value_name;

void bmp280P_reading();
void bmp280T_reading();

}  // namespace BMP280Sensor
