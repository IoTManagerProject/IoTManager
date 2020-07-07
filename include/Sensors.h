#pragma once

#include "Global.h"

#include "Sensors/OneWireBus.h"
#include "Sensors/AnalogSensor.h"
#include "Sensors/DallasSensor.h"
#include "Sensors/DHTSensor.h"
#include "Sensors/BMP280Sensor.h"
#include "Sensors/BME280Sensor.h"
#include "Sensors/UltrasonicSensor.h"

#define NUM_SENSORS 15

namespace Sensors {
void init();
void process();
void enable(size_t num, boolean enable = true);

}  // namespace Sensors
