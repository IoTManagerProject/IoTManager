#pragma once

#include "Global.h"

#include "Bus/OneWireBus.h"
#include "Sensor/AnalogSensor.h"
#include "Sensor/DallasSensor.h"
#include "Sensor/DhtSensor.h"
#include "Sensor/BMP280ESensor.h"
#include "Sensor/BMP280PSensor.h"
#include "Sensor/UltrasonicSensor.h"

#define NUM_SENSORS 15

namespace Sensors {

void init();

void enable(size_t num, boolean enable = true);

void ultrasonic_reading();


void dhtC_reading();
void dhtD_reading();
void dhtH_reading();
void dhtT_reading();
void dhtP_reading();

void dallas_reading();

void bmp280P_reading();
void bmp280T_reading();

void bme280A_reading();
void bme280T_reading();
void bme280P_reading();
void bme280H_reading();

}  // namespace Sensors
