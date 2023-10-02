#pragma once

#include "Global.h"

extern String jsonReadStrDoc(DynamicJsonDocument& doc, String name);
extern void jsonWriteStrDoc(DynamicJsonDocument& doc, String name, String value);

extern String jsonWriteStr(String& json, String name, String value, bool e = true);
extern String jsonWriteInt(String& json, String name, int value, bool e = true);
extern String jsonWriteFloat(String& json, String name, float value, bool e = true);
extern String jsonWriteBool(String& json, String name, boolean value, bool e = true);

extern bool jsonRead(const String& json, String key, long& value, bool e = true);
extern bool jsonRead(const String& json, String key, float& value, bool e = true);
extern bool jsonRead(const String& json, String key, String& value, bool e = true);
extern bool jsonRead(const String& json, String key, bool& value, bool e = true);
extern bool jsonRead(const String& json, String key, int& value, bool e = true);
extern bool jsonReadArray(const String& json, String key, std::vector<String>& jArray, bool e = true);

extern String jsonReadStr(const String& json, String name, bool e = true);
extern int jsonReadInt(const String& json, String name, bool e = true);
long int jsonReadLInt(const String& json, String name, bool e=true);
extern boolean jsonReadBool(const String& json, String name, bool e = true);

extern bool jsonWriteStr_(String& json, const String& name, const String& value, bool e = true);
extern bool jsonWriteBool_(String& json, const String& name, bool value, bool e = true);
extern bool jsonWriteInt_(String& json, const String& name, int value, bool e = true);
extern bool jsonWriteFloat_(String& json, const String& name, float value, bool e = true);
void writeUint8tValueToJsonString(uint8_t* payload, size_t length, size_t headerLenth, String& json);
extern bool jsonMergeObjects(String& json1, String& json2, bool e = true);
extern void jsonMergeDocs(JsonObject dest, JsonObjectConst src);
extern void jsonErrorDetected();
