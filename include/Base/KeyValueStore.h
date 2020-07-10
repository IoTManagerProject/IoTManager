#pragma once

#include "Arduino.h"

#include "Utils/JsonUtils.h"
#include "Utils/FileUtils.h"

class KeyValueStore {
   public:
    KeyValueStore() {
        _pool.reserve(1024);
    }

    String get() {
        return _pool;
    }

    String write(String name, String value) {
        return jsonWriteStr(_pool, name, value);
    }

    String read(const String obj) {
        return jsonReadStr(_pool, obj);
    }

    int readInt(const String name) {
        return jsonReadInt(_pool, name);
    }

    void writeInt(String name, int value) {
        jsonWriteInt(_pool, name, value);
    }

    void writeFloat(String name, float value) {
        jsonWriteFloat(_pool, name, value);
    }

    float readFloat(String name) {
        return jsonReadFloat(_pool, name);
    }

   protected:
    String _pool;
};

class KeyValueFile : public KeyValueStore {
   public:
    KeyValueFile(const char* filename) {
        strncpy(_filename, filename, sizeof(_filename));
    }

    void save() {
        writeFile(_filename, _pool);
    }

    void load() {
        if (readFile(_filename, _pool)) {
            _pool.replace(" ", "");
            _pool.replace("\r\n", "");
        } else {
            _pool = "{}";
        }
    }

   private:
    char _filename[33];
};