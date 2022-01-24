#include "ST7565.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <U8g2lib.h>
#include <vector>
#define STRHELPER(x) #x
#define TO_STRING_AUX(...) "" #__VA_ARGS__
#define TO_STRING(x) TO_STRING_AUX(x)

#define D_LOG(fmt, ...) Serial.printf((PGM_P)PSTR(fmt), ##__VA_ARGS__)

// Параметры
static constexpr uint8_t CHAR_HEIGHT = 10;
#define DESC_LEN 8
#define LINE_LEN 24
#define PARAM_LEN (LINE_LEN - DESC_LEN)

//

namespace ST7565
{

    // текущая
    size_t _page_n{0};
    size_t _line_n{0};

    U8G2_ST7565_ERC12864_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/D5, /* data=*/D7, /* cs=*/D8, /* dc=*/D2, /* reset=*/D3);

    bool _init{false};

    bool _firstRun{true};
    uint8_t _max_desc_width{0};
    uint32_t _lastResfresh{0};
    uint32_t _nextPage{0};
    bool _fullDraw{true};

    struct Line
    {
        // Ключ
        const String key;
        // Описание
        String descr;
        // Значение
        String value;
        // значение изменилось
        bool updated;

        Line(const String &key, const String &value = emptyString, const String &descr = emptyString) : key{key}
        {
            setDescr(descr.isEmpty() ? key.c_str() : descr.c_str());
            setValue(value.c_str());
        }

        bool setDescr(const char *str)
        {
            char buf[LINE_LEN + 1];
            snprintf(buf, sizeof(buf), "%s: ", str);
            if (!descr.equals(buf))
            {
                descr = buf;
                updated = true;
                return true;
            }
            return false;
        }

        bool setValue(const char *str)
        {
            if (!value.equals(str))
            {
                value = str;
                updated = true;
                return true;
            }
            return false;
        }

        size_t descrWidth()
        {
            return calcWidth(descr.c_str());
        }

        size_t valueWidth()
        {
            return calcWidth(value.c_str());
        }

        static size_t calcWidth(const char *str)
        {
            return u8g2.getUTF8Width(str);
        }
    };

    struct Display
    {
        static void init()
        {
            u8g2.begin();
            u8g2.setContrast(30);
            u8g2.setFont(u8g2_font_ncenB08_tr);
            u8g2.clearBuffer();
        }

        static uint8_t getHeight()
        {
            return u8g2.getDisplayHeight();
        }

        static uint8_t getLines()
        {
            return getHeight() / CHAR_HEIGHT;
        }

        static void clear()
        {
        }
    };

    std::vector<Line> _line;

    Line *findKey(const char *key)
    {
        Line *res = nullptr;
        for (size_t i = 0; i < _line.size(); i++)
        {
            if (_line.at(i).key.equalsIgnoreCase(key))
            {
                res = &_line.at(i);
                break;
            }
        }
        return res;
    }

    void calcMaxWidth()
    {
        _max_desc_width = 0;
        for (auto entry : _line)
        {
            auto width = entry.descrWidth();
            if (_max_desc_width < width)
                _max_desc_width = width;
        }
    }

    // Описание данных мы получаем по одному
    // {"key":"any109","addr":"","int":"10","c":"0","k":"30","val":"time","type":"ST7565","descr":"Temp"}
    // {"key":"any230","addr":"","int":"10","c":"0","k":"20","val":"upt","type":"ST7565","descr":"Hum"}
    // {"key":"any248","addr":"","int":"10","c":"0","k":"60","val":"time","type":"ST7565","descr":"Hum"}
    void load(String json)
    {
        StaticJsonBuffer<512> doc;
        JsonObject &root = doc.parseObject(json);
        auto key = root["key"].as<char *>();
        auto entry = findKey(key);
        if (!entry)
        {
            D_LOG("%s: not found", key);
            return;
        }
        auto descr = root["descr"].as<char *>();
        if (entry->setDescr(descr))
        {
            D_LOG("descr %s:%s\r\n", key, descr);
        }
    }

    // {"ip":"192.168.25.169","time":"23.01.22 01:18:44","weekday":"1","timenow":"01:18","upt":"00:00:18","any109":"0.00","any248":"0.00","any230":"0.00"}
    void fill(String json)
    {
        StaticJsonBuffer<512> doc;
        JsonObject &root = doc.parseObject(json);

        for (JsonObject::iterator it = root.begin(); it != root.end(); ++it)
        {
            auto key = (*it).key;
            auto value = (*it).value.as<char *>();
            auto entry = findKey(key);
            if (!entry)
                _line.push_back(Line(key, value));
            else
                entry->setValue(value);
        }

        calcMaxWidth();
    }

    void show(const String &data, const String &meta)
    {
        if (!_init)
        {
            Display::init();
            _init = true;
        }

        fill(data);

        load(meta);

        draw();
    }

    void draw()
    {
        if (_lastResfresh && (millis() < (_lastResfresh + PAGE_UPDATE_ms)))
            return;

        size_t page_lines = Display::getLines();

        // Количество страниц
        size_t page_cnt = _line.size() / page_lines;
        if (_line.size() % page_lines)
            page_cnt++;

        // Следующая страница
        if (millis() >= (_nextPage + PAGE_CHANGE_ms))
        {
            if (++_page_n >= page_cnt)
                _page_n = 0;
            _fullDraw = true;
            u8g2.clearDisplay();
            _nextPage = millis();
        }
        // Номер первой строки стрницы

        size_t line_first = _page_n * page_lines;
        // Номер последней строки стрницы
        size_t line_last = line_first + page_lines - 1;
        if (line_last > (_line.size() - 1))
            line_last = _line.size() - 1;

        Serial.printf("%d-%d\r\n", line_first, line_last);

        // Строка п/п на странице
        u8g2.clearBuffer();
        size_t page_line = 0;
        for (size_t n = line_first; n <= line_last; n++)
        {
            auto entry = &_line.at(n);
            Serial.printf("%s: %s%s\r\n", entry->key.c_str(), entry->descr.c_str(), entry->value.c_str());
            int y = CHAR_HEIGHT * (page_line++ + 1);
            u8g2.drawStr(0, y, entry->descr.c_str());
            int x = entry->descrWidth();
            u8g2.drawFrame(x, y, x + entry->valueWidth(), y + CHAR_HEIGHT);
            u8g2.drawStr(x, y, entry->value.c_str());
            entry->updated = false;
            }
        }

        u8g2.sendBuffer();

        _lastResfresh = millis();
    }
} // namespace ST7565
