#pragma once

#include <Arduino.h>

String jsonReadStr(String& json, String name);

int jsonReadInt(String& json, String name);

String jsonWriteStr(String& json, String name, String volume);

String jsonWriteInt(String& json, String name, int volume);

String jsonWriteFloat(String& json, String name, float volume);
