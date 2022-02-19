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
    rotation_t _rotate;
    uint8_t _contrast;
    String _font; 
   public:
    std::vector<DisplayPage> page;

   private:
    bool loadJson(String str) {
        StaticJsonBuffer<2048> doc;
        JsonObject& obj = doc.parseObject(str);
        if (!obj.success()) {
            D_LOG("parse: %s", str.c_str());
            return false;
        } else {
            _type = obj["type"].as<char*>();
            if (_type.isEmpty()) _type = "ST";
            _connection = obj["connection"].as<char*>();
            _update = obj["update"].as<int>();
            _font = obj["font"].as<char*>();
            _rotate = parse_rotation(obj["rotate"].success() ? obj["rotate"].as<int>() :DEFAULT_ROTATION);
            _contrast = parse_contrast(obj["contrast"].success() ? obj["contrast"].as<int>() : DEFAULT_CONTRAST);
            if (_update < DEFAULT_PAGE_UPDATE_ms) _update = DEFAULT_PAGE_UPDATE_ms;

            const JsonArray& pageArr = obj["page"].as<JsonArray>();
            for (auto it = pageArr.begin(); it != pageArr.end(); ++it) {
                auto pageObj = (*it);
                if (pageObj["key"].success()) {
                    // Страница с настройками умолчаниями (по заданным для дисплеля)
                    auto item = DisplayPage( pageObj["key"].as<char*>(), _update, _rotate, _font);
                    // Загрузка настроек страницы
                    item.load(pageObj);

                    page.push_back(item);
                }
            }
        }
        D_LOG("t: %s c: %s pc: %d", _type.c_str(), _connection.c_str(), page.size());
        return true;
    }

    bool loadFile(File& f) {
        if (f) {
            String buf = f.readString();
            f.close();           
            return  loadJson(buf);;
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

    bool init() {
        if (FileFS.exists(SETTINGS_FILE)) {
            auto f = FileFS.open(SETTINGS_FILE, FILE_READ);
            if (loadFile(f)) return true;
        }
        setDefault();
        return false;        
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

    uint8_t getContrast() {
        return _contrast;
    }

    rotation_t getRotation() {
        return _rotate;
    }
};
