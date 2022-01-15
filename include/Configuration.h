#pragma once
#include "Global.h"
#include "Classes/IoTSensor.h"

extern std::vector<IoTSensor*> iotSensors;  // вектор ссылок базового класса IoTSensor - список всех запущенных сенсоров

extern void configure(String path);