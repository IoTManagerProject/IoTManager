#pragma once

#include <Arduino.h>
extern float yourSensorReading(String type, String addr);

extern void HDC1080_init(String addr);
extern void AHTX0_init();
extern void SDS011_init();
extern void BH1750_init();
extern void NTC_init();//Вызов функции
extern void MHZ19_init();
extern void MAX6675_init();
extern void SHT30_init();
extern void HC4067_init();
extern void INA219_init();
extern void ADS1115_init();
extern void HX711_init();
extern void MCP23017_init();//Вызов функции




