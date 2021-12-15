#pragma once

#include <Arduino.h>
float yourSensorReading(String type, String addr);

void HDC1080_init(String &addr);
void AHTX0_init();
