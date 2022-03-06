#include "Utils/JsonUtils.h"

#include "Utils/FileUtils.h"

void onJsonWriteError(const String& err) {
    SerialPrint('E', "jsonWrite", err);
}

void onJsonReadError(const String& err) {
    SerialPrint('E', "jsonRead", err);
}

// new================================================================================
String jsonReadStrDoc(DynamicJsonDocument& doc, String name) {
    return doc[name].as<String>();
}

void jsonWriteStrDoc(DynamicJsonDocument& doc, String name, String value) {
    doc[name] = value;
}

// new==============================================================================
bool jsonRead(String& json, String key, String& value) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if(!error) {
        if (doc.containsKey(key)) {
            value = doc[key].as<String>();
            return true;
        } 
        onJsonReadError(key + " missing");
    } else {
        onJsonReadError(error.c_str());
    }
    return false;
}

bool jsonRead(String& json, String key, bool& value) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if(!error) {
        if (doc.containsKey(key)) {
            value = doc[key].as<bool>();
            return true;
        } 
        onJsonReadError(key + " missing");
    } else {
        onJsonReadError(error.c_str());
    }
    return false;
}

bool jsonRead(String& json, String key, int& value) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if(!error) {
        if (doc.containsKey(key)) {
            value = doc[key].as<int>();
            return true;
        } 
        onJsonReadError(key + " missing");
    } else {
        onJsonReadError(error.c_str());
    }
    return false;
}

// new==============================================================================
bool jsonWriteStr_(String& json, String key, String value) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) onJsonWriteError(error.c_str());    
    doc[key] = value;
    json = "";
    serializeJson(doc, json);
    return !error;
}

bool jsonWriteBool_(String& json, String key, bool value) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) onJsonWriteError(error.c_str());    
    doc[key] = value;
    json = "";
    serializeJson(doc, json);
    return !error;
}

bool jsonWriteInt_(String& json, String key, int value) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) onJsonWriteError(error.c_str());    
    doc[key] = value;
    json = "";
    serializeJson(doc, json);
    return !error;
}

bool jsonWriteFloat_(String& json, String key, float value) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);    
    if (error) onJsonWriteError(error.c_str());    
    doc[key] = value;
    json = "";
    serializeJson(doc, json);
    return !error;
}

// depricated======================================================================
String jsonReadStr(String& json, String name) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
   if (error) onJsonReadError(error.c_str());
    return doc[name].as<String>();
}

boolean jsonReadBool(String& json, String name) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
   if (error) onJsonReadError(error.c_str());
    return doc[name].as<bool>();
}

int jsonReadInt(String& json, String name) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) onJsonReadError(error.c_str());
    return doc[name].as<int>();
}

// depricated========================================================================
String jsonWriteStr(String& json, String name, String value) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) onJsonWriteError(error.c_str());
    doc[name] = value;
    json = "";
    serializeJson(doc, json);
    return json;
}

String jsonWriteBool(String& json, String name, boolean value) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) onJsonWriteError(error.c_str());
    doc[name] = value;
    json = "";
    serializeJson(doc, json);
    return json;
}

String jsonWriteInt(String& json, String name, int value) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) onJsonWriteError(error.c_str());
    doc[name] = value;
    json = "";
    serializeJson(doc, json);
    return json;
}

String jsonWriteFloat(String& json, String name, float value) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) onJsonWriteError(error.c_str());
    doc[name] = value;
    json = "";
    serializeJson(doc, json);
    return json;
}

