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
    String  _connection;

   private:
    void loadJson(String str) {
        StaticJsonBuffer<512> doc;
        JsonObject &obj = doc.parseObject(str);
        _type = obj["type"].as<char *>();
        _connection = obj["connection"].as<char *>();
        _page_count = obj["page_count"].as<int>();
    }

    bool loadFile(File &f) {
        if (f) {
            String buf = f.readString();
            loadJson(buf);
            return true;
        }
        return false;
    }

   public:
    void init() {
        if (FileFS.exists(SETTINGS_FILE)) {
            auto f = FileFS.open(SETTINGS_FILE, FILE_READ);
            loadFile(f);
            f.close();
        } else {
            auto f = FileFS.open(SETTINGS_FILE, FILE_WRITE);
            if (f) {
                f.println(SETTINGS_DEFAULT);
                f.close();
            }
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
};
