#pragma once

#include "GyverFilters.h"

#define TANK_LEVEL_SAMPLES 10
namespace Ultrasonic {
extern String levelPr_value_name;
extern String ultrasonicCm_value_name;

void ultrasonic_reading();
}