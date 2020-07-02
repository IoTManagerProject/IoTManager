#include "Options.h"

#include "Utils/JsonUtils.h"

namespace Options {

String optionJson = "{}";

String get() {
    return optionJson;
}

String write(String name, String value) {
    return jsonWriteStr(optionJson, name, value);
}

String read(const String& obj) {
    return jsonReadStr(optionJson, obj);
}

int readInt(const String& name) {
    return jsonReadInt(optionJson, name);
}

void writeInt(String name, int value) {
    jsonWriteInt(optionJson, name, value);
}

}  // namespace Options