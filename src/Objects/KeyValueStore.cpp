#include "Objects/KeyValueStore.h"

#include "Utils/JsonUtils.h"
#include "Utils/FileUtils.h"

KeyValueFile::KeyValueFile(const char* filename) {
    strncpy(_filename, filename, sizeof(_filename));
}

void KeyValueFile::save() {
    writeFile(_filename, _pool);
}

void KeyValueFile::load() {
    if (readFile(_filename, _pool)) {
        _pool.replace(" ", "");
        _pool.replace("\r\n", "");
    } else {
        _pool = "{}";
    }
}

KeyValueStore::KeyValueStore() {
    _pool.reserve(1024);
}

String KeyValueStore::get() {
    return _pool;
}

String KeyValueStore::write(String name, String value) {
    return jsonWriteStr(_pool, name, value);
}

String KeyValueStore::read(const String& obj) {
    return jsonReadStr(_pool, obj);
}

int KeyValueStore::readInt(const String& name) {
    return jsonReadInt(_pool, name);
}

void KeyValueStore::writeInt(String name, int value) {
    jsonWriteInt(_pool, name, value);
}

void KeyValueStore::writeFloat(String name, float value) {
    jsonWriteFloat(_pool, name, value);
}

float KeyValueStore::readFloat(String name) {
    return jsonReadFloat(_pool, name);
}
