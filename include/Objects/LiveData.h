#pragma once

#include "Global.h"

const String readLiveData(const String& obj) {
    return jsonReadStr(configLiveJson, obj);
}

const String writeLiveData(const String& obj, const String& value) {
    return jsonWriteStr(configLiveJson, obj, value);
}
