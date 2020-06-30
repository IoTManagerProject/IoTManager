#pragma once

#include "GyverFilters.h"

#define TANK_LEVEL_SAMPLES 10

extern GMedian<10, int> medianFilter;
extern String levelPr_value_name;
extern String ultrasonicCm_value_name;