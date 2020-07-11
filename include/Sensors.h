#pragma once

#include "Global.h"

#include "Sensors/OneWireBus.h"
#include "Sensors/AnalogSensor.h"
#include "Sensors/DallasSensors.h"
#include "Sensors/DHTSensor.h"
#include "Sensors/BMP280Sensor.h"
#include "Sensors/BME280Sensor.h"
#include "Sensors/UltrasonicSensor.h"

#define NUM_SENSORS 15

namespace Sensors {
AnalogSensor* add(const String& name, const String& pin);
void update();
SensorItem* last();
}  // namespace Sensors
