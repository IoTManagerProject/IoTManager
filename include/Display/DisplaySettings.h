#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#include "FileSystem.h"

static const char SETTINGS_FILE[] = "/display.json";
static const char SETTINGS_DEFAULT[] = "{\"type\": \"ST7556\", \"connection\": \"\", \"page_count\": 0}";

struct DisplaySettings {
   private:
    uint8_t _page_count{0};
    String _type;
    String _connection;
    uint16_t _page_change;
    uint16_t _page_update;
   private:
    void loadJson(String str) {
        StaticJsonBuffer<512> doc;
        JsonObject &obj = doc.parseObject(str);
        if (!obj.success()) {
            setDefault();
        } else {
            _type = obj["type"].as<char *>();
            if (_type.isEmpty()) _type = "ST";
            _connection = obj["connection"].as<char*>();
            _page_count = obj["page_count"].as<int>();
            _page_change = obj["page_change"].as<int>();
            _page_update = obj["page_update"].as<int>();             
            if (_page_update < 1000) _page_update = 1000;
            if (_page_change < _page_update) _page_change = _page_update;            
        }
        Serial.printf("t: %s c: %s pc: %d", _type.c_str(), _connection.c_str(), _page_count);
    }

    bool loadFile(File &f) {
        if (f) {
            String buf = f.readString();
            f.close();
            loadJson(buf);
            return true;
        }
        return false;
    }

   public:
    void setDefault() {
        auto f = FileFS.open(SETTINGS_FILE, FILE_WRITE);
        if (f) {
            f.println(SETTINGS_DEFAULT);
            f.close();
        }
    }

    void init() {
        if (FileFS.exists(SETTINGS_FILE)) {
            auto f = FileFS.open(SETTINGS_FILE, FILE_READ);
            loadFile(f);
        } else {
            setDefault();
        }
    }

    String getType() {
        return _type;
    }

    String getConnection() {
        return _connection;
    }

    bool isAutoPage() {
        return _page_count == 0;
    }

    uint8_t getPageCount() {
        return _page_count;
    }

    uint16_t getPageChange() {
        return _page_change;
    }

    uint16_t getPageUpdate() {
        return _page_update;
    }
};
