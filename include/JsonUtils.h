#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

String jsonReadStr(String& json, String name) {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(json);
    return root[name].as<String>();
}

int jsonReadInt(String& json, String name) {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(json);
    return root[name];
}

String jsonWriteStr(String& json, String name, String volume) {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(json);
    root[name] = volume;
    json = "";
    root.printTo(json);
    return json;
}

String jsonWriteInt(String& json, String name, int volume) {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(json);
    root[name] = volume;
    json = "";
    root.printTo(json);
    return json;
}

String jsonWriteFloat(String& json, String name, float volume) {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(json);
    root[name] = volume;
    json = "";
    root.printTo(json);
    return json;
}