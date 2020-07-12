#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <functional>
#include <Utils/FileUtils.h>

enum ValueType_t {
    VT_STRING,
    VT_FLOAT,
    VT_INT
};

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

    void setValue(const char* value) {
        _value = value;
        _type = VT_STRING;
    };

    void setValueInt(String value) {
        _value = value;
        _type = VT_INT;
    };

    void setValueFloat(float value) {
        _value = value;
        _type = VT_FLOAT;
    };
};

typedef std::function<void(const ValueType_t, const String&, const String&)> KeyValueHandler;

class KeyValueStore {
   protected:
    std::vector<KeyValue> _items;

   public:
    KeyValueStore() {
        _items.reserve(16);
    }

    void forEach(KeyValueHandler func) {
        for (auto item : _items) {
            func(item.getType(), item.getKey(), item.getValue());
        }
    }

    const String asJson() {
        DynamicJsonBuffer json;
        JsonObject& root = json.createObject();
        for (auto item : _items) {
            switch (item.getType()) {
                case VT_STRING:
                    root[item.getKey()] = item.getValue();
                    break;
                case VT_FLOAT:
                    root[item.getKey()] = atof(item.getValue().c_str());
                    break;
                case VT_INT:
                    root[item.getKey()] = atoi(item.getValue().c_str());
                default:
                    break;
            }
        }
        String buf;
        root.printTo(buf);
        return buf;
    }

    KeyValue* findKey(const String key) {
        for (size_t i = 0; i < _items.size(); i++) {
            if (_items.at(i).getKey() == key) {
                return &_items.at(i);
            }
        }
        return NULL;
    }

    void write(const String& key, const String& value) {
        auto item = findKey(key);
        if (item) {
            item->setValue(value.c_str());
        } else {
            _items.push_back(KeyValue{key.c_str(), value.c_str(), VT_STRING});
        }
    }

    const String read(const String& key) {
        auto item = findKey(key);
        return item ? item->getValue() : "";
    }

    int readInt(const String& key) {
        String buf = read(key);
        return buf.toInt();
    }

    void writeInt(const String& key, const String& value) {
        auto item = findKey(key);
        if (item) {
            item->setValueInt(value);
        } else {
            _items.push_back(KeyValue{key.c_str(), value.c_str(), VT_INT});
        }
    }

    void writeFloat(const String& key, float value) {
        auto item = findKey(key);
        if (item) {
            item->setValueFloat(value);
        } else {
            _items.push_back(KeyValue{key.c_str(), String(value, 4).c_str(), VT_FLOAT});
        }
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

    bool load() {
        bool res = false;
        DynamicJsonBuffer buf;
        auto file = LittleFS.open(_filename, "r");
        if (file) {
            JsonObject& root = buf.parse(file);
            for (JsonPair& p : root) {
                String key = p.key;
                String value{p.value.as<String>()};
                _items.push_back(KeyValue{key.c_str(), value.c_str(), VT_STRING});
            }
            res = true;
        }
        file.close();
        return res;
    }
};