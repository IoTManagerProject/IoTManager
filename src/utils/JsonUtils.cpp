#include "Utils/JsonUtils.h"

#include "Utils/FileUtils.h"

// depricated======================================================================
String jsonReadStr(String& json, String name) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) SerialPrint("EE", F("jsonRead"), error.f_str());
    return doc[name].as<String>();
}

boolean jsonReadBool(String& json, String name) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) SerialPrint("EE", F("jsonRead"), error.f_str());
    return doc[name].as<bool>();
}

int jsonReadInt(String& json, String name) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) SerialPrint("EE", F("jsonRead"), error.f_str());
    return doc[name].as<int>();
}

// new==============================================================================
bool jsonRead(String& json, String key, String& value) {
    bool ret = true;
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        SerialPrint("EE", F("jsonRead"), error.f_str());
        ret = false;
    } else if (!doc.containsKey(key)) {
        SerialPrint("EE", F("jsonRead"), key + " missing");
        ret = false;
    }
    value = doc[key].as<String>();
    return ret;
}

bool jsonRead(String& json, String key, bool& value) {
    bool ret = true;
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        SerialPrint("EE", F("jsonRead"), error.f_str());
        ret = false;
    } else if (!doc.containsKey(key)) {
        SerialPrint("EE", F("jsonRead"), key + " missing");
        ret = false;
    }
    value = doc[key].as<bool>();
    return ret;
}

bool jsonRead(String& json, String key, int& value) {
    bool ret = true;
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        SerialPrint("EE", F("jsonRead"), error.f_str());
        ret = false;
    } else if (!doc.containsKey(key)) {
        SerialPrint("EE", F("jsonRead"), key + " missing");
        ret = false;
    }
    value = doc[key].as<int>();
    return ret;
}
// depricated========================================================================
String jsonWriteStr(String& json, String name, String value) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) SerialPrint("EE", F("jsonWrite"), error.f_str());
    doc[name] = value;
    json = "";
    serializeJson(doc, json);
    return json;
}

String jsonWriteBool(String& json, String name, boolean value) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) SerialPrint("EE", F("jsonWrite"), error.f_str());
    doc[name] = value;
    json = "";
    serializeJson(doc, json);
    return json;
}

String jsonWriteInt(String& json, String name, int value) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) SerialPrint("EE", F("jsonWrite"), error.f_str());
    doc[name] = value;
    json = "";
    serializeJson(doc, json);
    return json;
}

String jsonWriteFloat(String& json, String name, float value) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) SerialPrint("EE", F("jsonWrite"), error.f_str());
    doc[name] = value;
    json = "";
    serializeJson(doc, json);
    return json;
}

// new==============================================================================
bool jsonWriteStr_(String& json, String key, String value) {
    bool ret = true;
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        SerialPrint("EE", F("jsonWrite"), error.f_str());
        ret = false;
    }
    doc[key] = value;
    json = "";
    serializeJson(doc, json);
    return ret;
}

bool jsonWriteBool_(String& json, String key, bool value) {
    bool ret = true;
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        SerialPrint("EE", F("jsonWrite"), error.f_str());
        ret = false;
    }
    doc[key] = value;
    json = "";
    serializeJson(doc, json);
    return ret;
}

bool jsonWriteInt_(String& json, String key, int value) {
    bool ret = true;
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        SerialPrint("EE", F("jsonWrite"), error.f_str());
        ret = false;
    }
    doc[key] = value;
    json = "";
    serializeJson(doc, json);
    return ret;
}

bool jsonWriteFloat_(String& json, String key, float value) {
    bool ret = true;
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        SerialPrint("EE", F("jsonWrite"), error.f_str());
        ret = false;
    }
    doc[key] = value;
    json = "";
    serializeJson(doc, json);
    return ret;
}

//=================================================================================
