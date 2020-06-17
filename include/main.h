
#pragma once

#include <Arduino.h>

void getMemoryLoad(String text);

extern String jsonReadStr(String& json, String name);
extern int jsonReadInt(String& json, String name);
extern String jsonWriteInt(String& json, String name, int volume);
extern String jsonWriteStr(String& json, String name, String volume);
extern void saveConfig();
extern String jsonWriteFloat(String& json, String name, float volume);

extern String getURL(String urls);

extern String writeFile(String fileName, String strings);
extern String readFile(String fileName, size_t len);
extern String addFile(String fileName, String strings);

extern String selectFromMarkerToMarker(String str, String found, int number);
extern String selectToMarker(String str, String found);
extern String deleteAfterDelimiter(String str, String found);
extern String deleteBeforeDelimiter(String str, String found);
extern String deleteBeforeDelimiterTo(String str, String found);

extern void servo_();
extern boolean isDigitStr(String str);
extern String jsonWriteStr(String& json, String name, String volume);

extern void led_blink(String satus);