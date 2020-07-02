#include "Runtime.h"

#include "Utils/JsonUtils.h"
#include "Utils/FileUtils.h"

namespace Runtime {

String runtimeJson = "{}";

String get() {
    return runtimeJson;
}

void save() {
    writeFile(DEVICE_RUNTIME_FILE, runtimeJson);
}

void load() {
    if (readFile(DEVICE_RUNTIME_FILE, runtimeJson)) {
        runtimeJson.replace(" ", "");
        runtimeJson.replace("\r\n", "");
    } else {
        runtimeJson = "{}";
    }
    Serial.println(runtimeJson);


    Serial.println(runtimeJson);
}

String write(String name, String value) {
    return jsonWriteStr(runtimeJson, name, value);
}

String read(const String& obj) {
    return jsonReadStr(runtimeJson, obj);
}

int readInt(const String& name) {
    return jsonReadInt(runtimeJson, name);
}

void writeInt(String name, int value) {
    jsonWriteInt(runtimeJson, name, value);
}

}  // namespace Runtime