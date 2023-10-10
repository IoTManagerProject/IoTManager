#include "utils/JsonUtils.h"
#include "utils/FileUtils.h"

// new================================================================================
String jsonReadStrDoc(DynamicJsonDocument &doc, String name) { return doc[name].as<String>(); }

void jsonWriteStrDoc(DynamicJsonDocument &doc, String name, String value) { doc[name] = value; }

// new==============================================================================
bool jsonRead(const String &json, String key, long &value, bool e) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        if (e) {
            SerialPrint("E", F("jsonRead"), error.f_str());
            jsonErrorDetected();
        }
        return false;
    } else if (!doc.containsKey(key)) {
        if (e) {
            SerialPrint("E", F("jsonRead"), key + " missing in " + json);
            jsonErrorDetected();
        }
        return false;
    }
    value = doc[key].as<long>();
    return true;
}

bool jsonRead(const String &json, String key, float &value, bool e) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        if (e) {
            SerialPrint("E", F("jsonRead"), error.f_str());
            jsonErrorDetected();
        }
        return false;
    } else if (!doc.containsKey(key)) {
        if (e) {
            SerialPrint("E", F("jsonRead"), key + " missing in " + json);
            jsonErrorDetected();
        }
        return false;
    }
    value = doc[key].as<float>();
    return true;
}

bool jsonRead(const String &json, String key, String &value, bool e) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        if (e) {
            SerialPrint("E", F("jsonRead"), error.f_str());
            jsonErrorDetected();
        }
        return false;
    } else if (!doc.containsKey(key)) {
        if (e) {
            SerialPrint("E", F("jsonRead"), key + " missing in " + json);
            jsonErrorDetected();
        }
        return false;
    }
    value = doc[key].as<String>();
    return true;
}

bool jsonRead(const String &json, String key, bool &value, bool e) {
    int lvalue = value;
    bool ret = jsonRead(json, key, lvalue, e);
    value = lvalue;
    return ret;
}

bool jsonRead(const String &json, String key, int &value, bool e) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        if (e) {
            SerialPrint("E", F("jsonRead"), error.f_str());
            jsonErrorDetected();
        }
        return false;
    } else if (!doc.containsKey(key)) {
        if (e) {
            SerialPrint("E", F("jsonRead"), key + " missing in " + json);
            jsonErrorDetected();
        }
        return false;
    }
    value = doc[key].as<int>();
    return true;
}

bool jsonReadArray(const String &json, String key, std::vector<String> &jArray, bool e) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        if (e) {
            SerialPrint("E", F("jsonReadArray"), error.f_str());
            jsonErrorDetected();
        }
        return false;
    } else if (!doc.containsKey(key)) {
        if (e) {
            SerialPrint("E", F("jsonReadArray"), key + " missing in " + json);
            jsonErrorDetected();
        }
        return false;
    }

    if (doc[key].is<JsonArray>()) {
        for (int8_t i = 0; i < doc[key].size(); i++) jArray.push_back(doc[key][i].as<String>());

    } else {
        jArray.push_back(doc[key].as<String>());
    }

    return true;
}

// new==============================================================================
bool jsonWriteStr_(String &json, const String &key, const String &value, bool e) {
    bool ret = true;
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        if (e) {
            SerialPrint("E", F("jsonWrite"), error.f_str());
            jsonErrorDetected();
        }
        ret = false;
    }
    doc[key] = value;
    json = "";
    serializeJson(doc, json);
    return ret;
}

bool jsonWriteBool_(String &json, const String &key, bool value, bool e) {
    bool ret = true;
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        if (e) {
            SerialPrint("E", F("jsonWrite"), error.f_str());
            jsonErrorDetected();
        }
        ret = false;
    }
    doc[key] = value;
    json = "";
    serializeJson(doc, json);
    return ret;
}

bool jsonWriteInt_(String &json, const String &key, int value, bool e) {
    bool ret = true;
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        if (e) {
            SerialPrint("E", F("jsonWrite"), error.f_str());
            jsonErrorDetected();
        }
        ret = false;
    }
    doc[key] = value;
    json = "";
    serializeJson(doc, json);
    return ret;
}

bool jsonWriteFloat_(String &json, const String &key, float value, bool e) {
    bool ret = true;
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        if (e) {
            SerialPrint("E", F("jsonWrite"), error.f_str());
            jsonErrorDetected();
        }
        ret = false;
    }
    doc[key] = value;
    json = "";
    serializeJson(doc, json);
    return ret;
}

void writeUint8tValueToJsonString(uint8_t *payload, size_t length, size_t headerLenth, String &json) {
    String payloadStr;
    payloadStr.reserve(length + 1);
    for (size_t i = headerLenth; i < length; i++) {
        payloadStr += (char)payload[i];
    }
    jsonMergeObjects(json, payloadStr);
}

bool jsonMergeObjects(String &json1, String &json2, bool e) {
    bool ret = true;
    DynamicJsonDocument doc1(JSON_BUFFER_SIZE);
    DeserializationError error1 = deserializeJson(doc1, json1);
    DynamicJsonDocument doc2(JSON_BUFFER_SIZE);
    DeserializationError error2 = deserializeJson(doc2, json2);
    jsonMergeDocs(doc1.as<JsonObject>(), doc2.as<JsonObject>());
    if (error1 || error2) {
        if (e) {
            SerialPrint("E", F("json"), "jsonMergeObjects error");
            jsonErrorDetected();
        }
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
String jsonReadStr(const String &json, String name, bool e) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        if (e) {
            SerialPrint("E", F("jsonRead"), error.f_str());
            jsonErrorDetected();
        }
    }
    return doc[name].as<String>();
}

boolean jsonReadBool(const String &json, String name, bool e) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        if (e) {
            SerialPrint("E", F("jsonRead"), error.f_str());
            jsonErrorDetected();
        }
    }
    return doc[name].as<bool>();
}

int jsonReadInt(const String &json, String name, bool e) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        if (e) {
            SerialPrint("E", F("jsonRead"), error.f_str());
            jsonErrorDetected();
        }
    }
    return doc[name].as<int>();
}

long int jsonReadLInt(const String &json, String name, bool e) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        if (e) {
            SerialPrint("E", F("jsonRead"), error.f_str());
            jsonErrorDetected();
        }
    }
    return doc[name].as<long int>();
}

// depricated========================================================================
String jsonWriteStr(String &json, String name, String value, bool e) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        if (e) {
            SerialPrint("E", F("jsonWrite"), error.f_str());
            jsonErrorDetected();
        }
    }
    doc[name] = value;
    json = "";
    serializeJson(doc, json);
    return json;
}

String jsonWriteBool(String &json, String name, boolean value, bool e) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        if (e) {
            SerialPrint("E", F("jsonWrite"), error.f_str());
            jsonErrorDetected();
        }
    }
    doc[name] = value;
    json = "";
    serializeJson(doc, json);
    return json;
}

String jsonWriteInt(String &json, String name, int value, bool e) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        if (e) {
            SerialPrint("E", F("jsonWrite"), error.f_str());
            jsonErrorDetected();
        }
    }
    doc[name] = value;
    json = "";
    serializeJson(doc, json);
    return json;
}

String jsonWriteFloat(String &json, String name, float value, bool e) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        if (e) {
            SerialPrint("E", F("jsonWrite"), error.f_str());
            jsonErrorDetected();
        }
    }
    doc[name] = value;
    json = "";
    serializeJson(doc, json);
    return json;
}

void jsonErrorDetected() {
    // пример как отправить ошибку с количеством
    // jsonWriteInt(errorsHeapJson, F("jse2"), 1);
    // int number = jsonReadInt(errorsHeapJson, F("jse2n"));
    // number++;
    // jsonWriteInt(errorsHeapJson, F("jse2n"), number);
}