#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <functional>
#include <Utils/FileUtils.h>
#include "Value.h"

class KeyValue {
   private:
    String _key;
    String _value;
    ValueType_t _type;

   public:
    KeyValue(const char* key, const char* value, ValueType_t type) : _key{key},
                                                                     _value{value},
                                                                     _type{type} {};
    ValueType_t getType() const {
        return _type;
    }

    const String getKey() const {
        return _key;
    }

    const String getValue() const {
        return _value;
    }

    void setValue(ValueType_t type, const String& value) {
        _type = type;
        _value = value.c_str();
    };

    void setValue(const char* value) {
        _value = value;
        _type = VT_STRING;
    };

    void setValueInt(int value) {
        _value = value;
        _type = VT_INT;
    };

    void setValueFloat(float value) {
        _value = value;
        _type = VT_FLOAT;
    };
};

typedef std::function<void(KeyValue*)> KeyValueHandler;

class KeyValueStore {
   protected:
    std::vector<KeyValue*> _items;

   public:
    KeyValueStore(){};

    KeyValueStore(const char* str) {
        _items.reserve(8);
        loadString(str);
    }

    ~KeyValueStore() {
        _items.clear();
    }

    void loadString(const char* str) {
        DynamicJsonBuffer buf;
        JsonObject& root = buf.parse(str);
        for (JsonPair& p : root) {
            String key = p.key;
            String value{p.value.as<String>()};
            _items.push_back(new KeyValue{key.c_str(), value.c_str(), VT_STRING});
        }
    }

    void clear() {
        _items.clear();
    }

    void forEach(KeyValueHandler func) {
        for (auto item : _items) {
            func(item);
        }
    }

    const String asJson() {
        DynamicJsonBuffer json;
        JsonObject& root = json.createObject();
        for (auto item : _items) {
            switch (item->getType()) {
                case VT_STRING:
                    root[item->getKey()] = item->getValue();
                    break;
                case VT_FLOAT:
                    root[item->getKey()] = atof(item->getValue().c_str());
                    break;
                case VT_INT:
                    root[item->getKey()] = atoi(item->getValue().c_str());
                default:
                    break;
            }
        }
        String buf;
        root.printTo(buf);
        return buf;
    }

    void erase(const String key) {
        for (size_t i = 0; i < _items.size(); i++) {
            if (_items.at(i)->getKey() == key) {
                _items.erase(_items.begin() + i);
                break;
            }
        }
    }

    KeyValue* findKey(const String key) {
        for (size_t i = 0; i < _items.size(); i++) {
            if (_items.at(i)->getKey() == key) {
                return _items.at(i);
            }
        }
        return NULL;
    }

    void write(const String& key, int value) {
        write(key, String(value, DEC), VT_INT);
    }

    void write(const String& key, const String& value, ValueType_t type = VT_STRING) {
        auto item = findKey(key);
        if (item) {
            item->setValue(type, value);
        } else {
            _items.push_back(new KeyValue{key.c_str(), value.c_str(), type});
        }
    }

    const String read(const String& key) {
        auto item = findKey(key);
        return item ? item->getValue() : "";
    }

    bool read(const String& key, String& value, ValueType_t& type) {
        auto item = findKey(key);
        if (item) {
            type = item->getType();
            value = item->getValue();
            return true;
        }
        return false;
    }

    int readInt(const String& key) {
        String buf = read(key);
        return buf.toInt();
    }

    float readFloat(const String& key) {
        String buf = read(key);
        return buf.toFloat();
    }
};

class KeyValueFile : public KeyValueStore {
   private:
    char _filename[32];

   public:
    KeyValueFile() {
    }

    KeyValueFile(const char* filename) {
        if (!(filename[0] == '/')) {
            _filename[0] = '/';
        };
        strncat(_filename, filename, sizeof(_filename));
    }

    bool save() {
        bool res = false;
        String buf = asJson();
        auto file = LittleFS.open(_filename, "w");
        if (file) {
            file.print(buf);
            res = true;
        }
        file.close();
        return res;
    }

    void reload() {
        loadFile(_filename);
    }

    bool loadFile(const char* filename) {
        bool res = false;
        DynamicJsonBuffer buf;
        auto file = LittleFS.open(filename, "r");
        if (file) {
            JsonObject& root = buf.parse(file);
            for (JsonPair& p : root) {
                String key = p.key;
                String value{p.value.as<String>()};
                _items.push_back(new KeyValue{key.c_str(), value.c_str(), VT_STRING});
            }
            res = true;
        }
        file.close();
        return res;
    }
};