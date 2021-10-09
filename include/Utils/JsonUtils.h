#pragma once

#include <Arduino.h>

String jsonReadStr(String& json, String name);
int jsonReadInt(String& json, String name);
boolean jsonReadBool(String& json, String name);

String jsonWriteStr(String& json, String name, String value);
String jsonWriteInt(String& json, String name, int value);
String jsonWriteFloat(String& json, String name, float value);
String jsonWriteBool(String& json, String name, boolean value);

bool jsonRead(String& json, String key, String& value);
bool jsonRead(String& json, String key, bool& value);
bool jsonRead(String& json, String key, int& value);

bool jsonWrite(String& json, String name, String value);
bool jsonWrite(String& json, String name, bool value);
bool jsonWrite(String& json, String name, int value);
bool jsonWrite(String& json, String name, float value);

void saveConfig();
void saveStore();