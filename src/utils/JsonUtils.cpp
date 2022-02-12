#include "Utils/JsonUtils.h"
#include "Utils/FileUtils.h"

// new================================================================================
String jsonReadStrDoc(DynamicJsonDocument& doc, String name) {
    return doc[name].as<String>();
}

void jsonWriteStrDoc(DynamicJsonDocument& doc, String name, String value) {
    doc[name] = value;
}

// new==============================================================================
bool jsonRead(String& json, String key, unsigned long& value) {
    bool ret = true;
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        SerialPrint("EE", F("jsonRead"), error.f_str());
        jsonErrorDetected();
        ret = false;
    } else if (!doc.containsKey(key)) {
        SerialPrint("EE", F("jsonRead"), key + " missing");
        jsonErrorDetected();
        ret = false;
    }
    value = doc[key].as<unsigned long>();
    return ret;
}

bool jsonRead(String& json, String key, float& value) {
    bool ret = true;
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        SerialPrint("EE", F("jsonRead"), error.f_str());
        jsonErrorDetected();
        ret = false;
    } else if (!doc.containsKey(key)) {
        SerialPrint("EE", F("jsonRead"), key + " missing");
        jsonErrorDetected();
        ret = false;
    }
    value = doc[key].as<float>();
    return ret;
}

bool jsonRead(String& json, String key, String& value) {
    bool ret = true;
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        SerialPrint("EE", F("jsonRead"), error.f_str());
        jsonErrorDetected();
        ret = false;
    } else if (!doc.containsKey(key)) {
        SerialPrint("EE", F("jsonRead"), key + " missing");
        jsonErrorDetected();
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
        jsonErrorDetected();
        ret = false;
    } else if (!doc.containsKey(key)) {
        SerialPrint("EE", F("jsonRead"), key + " missing");
        jsonErrorDetected();
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
        jsonErrorDetected();
        ret = false;
    } else if (!doc.containsKey(key)) {
        SerialPrint("EE", F("jsonRead"), key + " missing");
        jsonErrorDetected();
        ret = false;
    }
    value = doc[key].as<int>();
    return ret;
}

// new==============================================================================
bool jsonWriteStr_(String& json, String key, String value) {
    bool ret = true;
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        SerialPrint("EE", F("jsonWrite"), error.f_str());
        jsonErrorDetected();
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
        jsonErrorDetected();
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
        jsonErrorDetected();
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
        jsonErrorDetected();
        ret = false;
    }
    doc[key] = value;
    json = "";
    serializeJson(doc, json);
    return ret;
}

void writeUint8tValueToJsonString(uint8_t* payload, size_t length, size_t headerLenth, String& json) {
    String payloadStr;
    payloadStr.reserve(length + 1);
    for (size_t i = headerLenth; i < length; i++) {
        payloadStr += (char)payload[i];
    }
    jsonMerge(json, payloadStr);
}

bool jsonMerge(String& json1, String& json2) {
    bool ret = true;
    DynamicJsonDocument doc1(JSON_BUFFER_SIZE);
    DeserializationError error1 = deserializeJson(doc1, json1);
    DynamicJsonDocument doc2(JSON_BUFFER_SIZE);
    DeserializationError error2 = deserializeJson(doc2, json2);
    jsonMergeDocs(doc1.as<JsonObject>(), doc2.as<JsonObject>());
    if (error1 || error2) {
        SerialPrint("EE", F("json"), "jsonMerge error");
        jsonErrorDetected();
        ret = false;
    }
    json1 = "";
    serializeJson(doc1, json1);
    return ret;
}

void jsonMergeDocs(JsonObject dest, JsonObjectConst src) {
    for (auto kvp : src) {
        dest[kvp.key()] = kvp.value();
    }
}

// depricated======================================================================
String jsonReadStr(String& json, String name) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        SerialPrint("EE", F("jsonRead"), error.f_str());
        jsonErrorDetected();
    }
    return doc[name].as<String>();
}

boolean jsonReadBool(String& json, String name) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        SerialPrint("EE", F("jsonRead"), error.f_str());
        jsonErrorDetected();
    }
    return doc[name].as<bool>();
}

int jsonReadInt(String& json, String name) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        SerialPrint("EE", F("jsonRead"), error.f_str());
        jsonErrorDetected();
    }
    return doc[name].as<int>();
}

// depricated========================================================================
String jsonWriteStr(String& json, String name, String value) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        SerialPrint("EE", F("jsonWrite"), error.f_str());
        jsonErrorDetected();
    }
    doc[name] = value;
    json = "";
    serializeJson(doc, json);
    return json;
}

String jsonWriteBool(String& json, String name, boolean value) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        SerialPrint("EE", F("jsonWrite"), error.f_str());
        jsonErrorDetected();
    }
    doc[name] = value;
    json = "";
    serializeJson(doc, json);
    return json;
}

String jsonWriteInt(String& json, String name, int value) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        SerialPrint("EE", F("jsonWrite"), error.f_str());
        jsonErrorDetected();
    }
    doc[name] = value;
    json = "";
    serializeJson(doc, json);
    return json;
}

String jsonWriteFloat(String& json, String name, float value) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        SerialPrint("EE", F("jsonWrite"), error.f_str());
        jsonErrorDetected();
    }
    doc[name] = value;
    json = "";
    serializeJson(doc, json);
    return json;
}

void jsonErrorDetected() {
    jsonWriteInt(errorsHeapJson, F("jse2"), 1);
    int number = jsonReadInt(errorsHeapJson, F("jse2n"));
    number++;
    jsonWriteInt(errorsHeapJson, F("jse2n"), number);
}