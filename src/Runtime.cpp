#include "Runtime.h"

#include "Utils/JsonUtils.h"

namespace Runtime {

String runtimeJson = "{}";

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