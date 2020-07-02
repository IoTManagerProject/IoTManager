#include "Sensors.h"

#include "Utils/PrintMessage.h"

const static char *MODULE = "Sensors";

namespace Sensors {

boolean sensor_enabled[NUM_SENSORS] = {false};

void init() {
    for (size_t i = 0; i < NUM_SENSORS; i++) {
        enable(i, false);
    }
}

void enable(size_t num, boolean value) {
    sensor_enabled[num] = value;
}

void process() {
    AnalogSensor::loop();

    if (sensor_enabled[0])
        Ultrasonic::ultrasonic_reading();
    if (sensor_enabled[3])
        Dallas::dallas_reading();
    if (sensor_enabled[4])
        DHTSensor::dhtT_reading();
    if (sensor_enabled[5])
        DHTSensor::dhtH_reading();
    if (sensor_enabled[6])
        DHTSensor::dhtP_reading();
    if (sensor_enabled[7])
        DHTSensor::dhtC_reading();
    if (sensor_enabled[8])
        DHTSensor::dhtD_reading();
    if (sensor_enabled[9])
        BMP280Sensor::bmp280T_reading();
    if (sensor_enabled[10])
        BMP280Sensor::bmp280P_reading();
    if (sensor_enabled[11])
        BME280Sensor::bme280T_reading();
    if (sensor_enabled[12])
        BME280Sensor::bme280P_reading();
    if (sensor_enabled[13])
        BME280Sensor::bme280H_reading();
    if (sensor_enabled[14])
        BME280Sensor::bme280A_reading();
}

}  // namespace Sensors