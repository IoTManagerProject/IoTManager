#pragma once

#include <Arduino.h>

float yourSensorReading(String type, String addr);

void HDC1080_init(String addr);
void AHTX0_init();
void LCD_init();
void BH1750_init();

unsigned long getValue(byte packet[]);
void sendRequest(byte packet[]);
void SSD_1306_init();
void MCP23017_init();
void ST7565_init();
