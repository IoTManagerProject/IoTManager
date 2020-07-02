#include "LiveData.h"

#include "Utils/JsonUtils.h"

namespace LiveData {
String liveJson = "{}";

String get() {
    return liveJson;
}

String read(const String& obj) {
    return jsonReadStr(liveJson, obj);
}

int readInt(const String& name) {
    return jsonReadInt(liveJson, name);
}

String write(String name, String value) {
    return jsonWriteStr(liveJson, name, value);
}

void writeInt(String name, int value) {
    jsonWriteInt(liveJson, name, value);
}

void writeFloat(String name, float value) {
    jsonWriteFloat(liveJson, name, value);
}
}  // namespace LiveData