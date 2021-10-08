#include "Utils/JsonUtils.h"

#include <ArduinoJson.h>

#include "Global.h"
#include "Utils/FileUtils.h"

//String jsonReadStr(String& json, String name) {
//    DynamicJsonBuffer jsonBuffer;
//    JsonObject& root = jsonBuffer.parseObject(json);
//    return root[name].as<String>();
//}
//
//boolean jsonReadBool(String& json, String name) {
//    DynamicJsonBuffer jsonBuffer;
//    JsonObject& root = jsonBuffer.parseObject(json);
//    return root[name].as<boolean>();
//}
//
//int jsonReadInt(String& json, String name) {
//    DynamicJsonBuffer jsonBuffer;
//    JsonObject& root = jsonBuffer.parseObject(json);
//    return root[name];
//}
//
//String jsonWriteStr(String& json, String name, String value) {
//    DynamicJsonBuffer jsonBuffer;
//    JsonObject& root = jsonBuffer.parseObject(json);
//    root[name] = value;
//    json = "";
//    root.printTo(json);
//    return json;
//}
//
//String jsonWriteBool(String& json, String name, boolean value) {
//    return jsonWriteStr(json, name, value ? "1" : "0");
//}
//
//String jsonWriteInt(String& json, String name, int value) {
//    DynamicJsonBuffer jsonBuffer;
//    JsonObject& root = jsonBuffer.parseObject(json);
//    root[name] = value;
//    json = "";
//    root.printTo(json);
//    return json;
//}
//
//String jsonWriteFloat(String& json, String name, float value) {
//    DynamicJsonBuffer jsonBuffer;
//    JsonObject& root = jsonBuffer.parseObject(json);
//    root[name] = value;
//    json = "";
//    root.printTo(json);
//    return json;
//}
//
//void saveConfig() {
//    writeFile(String("config.json"), configSetupJson);
//}
//
//void saveStore() {
//    writeFile(String("store.json"), configStoreJson);
//}

String jsonReadStr(String& json, String name) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    deserializeJson(doc, json);
    return doc[name].as<String>();
}

boolean jsonReadBool(String& json, String name) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    deserializeJson(doc, json);
    return doc[name].as<bool>();
}

int jsonReadInt(String& json, String name) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    deserializeJson(doc, json);
    return doc[name].as<int>();
}

String jsonWriteStr(String& json, String name, String value) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    deserializeJson(doc, json);
    doc[name] = value;
    return json;
}

String jsonWriteBool(String& json, String name, boolean value) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    deserializeJson(doc, json);
    doc[name] = value;
    return json;
}

String jsonWriteInt(String& json, String name, int value) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    deserializeJson(doc, json);
    doc[name] = value;
    return json;
}

String jsonWriteFloat(String& json, String name, float value) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    deserializeJson(doc, json);
    doc[name] = value;
    return json;
}

void saveConfig() {
    writeFile(String("config.json"), configSetupJson);
}

void saveStore() {
    writeFile(String("store.json"), configStoreJson);
}