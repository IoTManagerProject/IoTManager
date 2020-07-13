#pragma once

#include "Global.h"

#include "Sensors/OneWireBus.h"
#include "Sensors/AnalogSensor.h"
#include "Sensors/DallasSensor.h"
#include "Sensors/DHTSensor.h"
#include "Sensors/BMP280Sensor.h"
#include "Sensors/BME280Sensor.h"
#include "Sensors/UltrasonicSensor.h"

enum SensorType_t {
    SENSOR_ADC,
    SENSOR_DALLAS
};

namespace Sensors {

BaseSensor* add(SensorType_t type, const String& name, const String& assign);

void update();
}  // namespace Sensors
