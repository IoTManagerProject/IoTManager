#pragma once

#include <Arduino.h>

String jsonReadStr(String& json, String name);

int jsonReadInt(String& json, String name);

boolean jsonReadBool(String& json, String name);

String jsonWriteStr(String& json, String name, String value);

String jsonWriteInt(String& json, String name, int value);

String jsonWriteFloat(String& json, String name, float value);

String jsonWriteBool(String& json, String name, boolean value);