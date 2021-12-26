#pragma once

#include <Arduino.h>
extern float yourSensorReading(String type, String addr);

extern void HDC1080_init(String addr);
extern void AHTX0_init();
extern void LCD_init();
extern void BH1750_init();

extern unsigned long getValue(byte packet[]);
extern void sendRequest(byte packet[]);