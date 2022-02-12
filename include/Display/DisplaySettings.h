#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#include "DisplayTypes.h"

#include "FileSystem.h"

struct DisplaySettings {
   private:    
    String _type;
    String _connection;
    uint16_t _update;
public:
    std::vector<DisplayPage> page;
   private:
    void loadJson(String str) {
        StaticJsonBuffer<512> doc;
        JsonObject &obj = doc.parseObject(str);
        if (!obj.success()) {
            D_LOG("parse: %s", str.c_str());
        } else {
            _type = obj["type"].as<char *>();
            if (_type.isEmpty()) _type = "ST";
            _connection = obj["connection"].as<char*>();
            _update = obj["update"].as<int>();
            if (_update < DEFAULT_PAGE_UPDATE_ms) _update = DEFAULT_PAGE_UPDATE_ms;
            const JsonArray& pageArr = obj["page"].as<JsonArray>();
            for (auto it = pageArr.begin(); it != pageArr.end(); ++it)
            {   
                auto pageObj = (*it);                
                page.push_back({
                    pageObj["time"].as<uint16_t>(), 
                    pageObj["item"].as<char*>()
                    });
            }                    
        }
         D_LOG("t: %s c: %s pc: %d", _type.c_str(), _connection.c_str(), page.size());
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
        return !getPageCount();
    }

    uint8_t getPageCount() {
        return page.size();
    }

    DisplayPage* getPage(uint8_t index) {
        return &page.at(index);
    }

    uint16_t getDisplayUpdate() {
        return _update;
    }
};
