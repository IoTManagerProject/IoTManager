#pragma once

#include "Global.h"

extern String jsonReadStrDoc(DynamicJsonDocument& doc, String name);
extern void jsonWriteStrDoc(DynamicJsonDocument& doc, String name, String value);

extern String jsonWriteStr(String& json, String name, String value);
extern String jsonWriteInt(String& json, String name, int value);
extern String jsonWriteFloat(String& json, String name, float value);
extern String jsonWriteBool(String& json, String name, boolean value);

extern bool jsonRead(String& json, String key, unsigned long& value);
extern bool jsonRead(String& json, String key, float& value);
extern bool jsonRead(String& json, String key, String& value);
extern bool jsonRead(String& json, String key, bool& value);
extern bool jsonRead(String& json, String key, int& value);

extern String jsonReadStr(String& json, String name);
extern int jsonReadInt(String& json, String name);
extern boolean jsonReadBool(String& json, String name);

extern bool jsonWriteStr_(String& json, String name, String value);
extern bool jsonWriteBool_(String& json, String name, bool value);
extern bool jsonWriteInt_(String& json, String name, int value);
extern bool jsonWriteFloat_(String& json, String name, float value);
void writeUint8tValueToJsonString(uint8_t* payload, size_t length, size_t headerLenth, String& json);
extern bool jsonMerge(String& json1, String& json2);
extern void jsonMergeDocs(JsonObject dest, JsonObjectConst src);
extern void jsonErrorDetected();
