#include "ST7565.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <U8g2lib.h>
#include <vector>

#define STRHELPER(x) #x
#define TO_STRING_AUX(...) "" #__VA_ARGS__
#define TO_STRING(x) TO_STRING_AUX(x)

// Параметры
static constexpr uint8_t LINE_NUMS = 6;
#define PAGE_UPDATE_ms 1000
#define DESC_LEN 8
#define LINE_LEN 24
#define PARAM_LEN (LINE_LEN - DESC_LEN)

static constexpr uint8_t CHAR_HEIGHT = 10;
//

namespace ST7565
{
    struct Line
    {
        // Ключ
        String key;
        // Представление на экране
        String render;
        // Значение
        String value;
        // Длина в пикселях
        uint8_t _key_width;
    };

    std::vector<Line> _line;
    // текущая
    size_t _page_n{0};
    size_t _line_n{0};

    U8G2_ST7565_ERC12864_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/D5, /* data=*/D7, /* cs=*/D8, /* dc=*/D2, /* reset=*/D3);

    bool _init{false};

    bool _firstRun{true};
    uint8_t _max_key_width{0};
    uint32_t _lastResfresh{0};
    void init()
    {

        u8g2.begin();
        u8g2.setContrast(30);
        u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font

        u8g2.clearBuffer();
    }

    void clear()
    {
    }

    char *renderKey(char *buf, size_t buf_size, const char *str, size_t str_len)
    {
        char fmt[LINE_LEN];
        snprintf(fmt, sizeof(fmt), "%%%ds: ", str_len);
        snprintf(buf, buf_size, fmt, str);
        return buf;
    }

    char *renderValue(char *buf, size_t buf_size, const char *str, size_t str_len)
    {
        char fmt[LINE_LEN];
        snprintf(fmt, sizeof(fmt), "%%%ds", str_len);
        snprintf(buf, buf_size, fmt, str);
        return buf;
    }

    size_t calcWidth(const char *str)
    {
        return u8g2.getUTF8Width(str);
    }

    size_t showParamKey(const char *str, size_t max_len, uint8_t x, uint8_t y)
    {
        char buf[LINE_LEN + 1];
        renderKey(buf, sizeof(buf), str, max_len);
        Serial.print(buf);
        return u8g2.drawStr(x, y, buf);
    }

    size_t showParamValue(const char *str, size_t max_len, uint8_t x, uint8_t y)
    {
        // char buf[LINE_LEN + 1];
        // renderValue(buf, sizeof(buf), str, max_len);
        return u8g2.drawStr(x, y, str);
    }

    bool findKey(const char *key)
    {
        for (auto entry : _line)
        {
            if (entry.key.equalsIgnoreCase(key))
                return true;
        }
        return false;
    }

    // У нас сразу есть даные чтобы выводить их
    // {"ip":"192.168.25.169","time":"23.01.22 01:18:44","weekday":"1","timenow":"01:18","upt":"00:00:18","any109":"0.00","any248":"0.00","any230":"0.00"}
    void show(String json)
    {
        Serial.println(json);

        if (!_init)
        {
            init();
            _init = true;
        }

        StaticJsonBuffer<512> doc;
        JsonObject &root = doc.parseObject(json);

        // Обновим параметры строк их имена
        for (JsonObject::iterator it = root.begin(); it != root.end(); ++it)
        {
            auto key = (*it).key;
            if (!findKey(key))
            {
                char buf[LINE_LEN + 1];
                renderKey(buf, sizeof(buf), key, DESC_LEN);
                uint8_t val = calcWidth(buf);
                if (_max_key_width < val)
                    _max_key_width = val;
                _line.push_back(Line{key, buf, emptyString, val});
            }
        }

        if (_firstRun)
        {
            size_t n{0};
            for (auto entry : _line)
            {
                if (++n >= LINE_NUMS)
                    break;
                u8g2.drawStr(
                    _max_key_width - entry._key_width,
                    CHAR_HEIGHT * (n + 1),
                    entry.render.c_str());
            }
            _firstRun = false;
        }

        // Тут у нас динамически пополняемый массив
        //  всех параметров их представлений на дисплее
        // по которому мы обходим полученый json в поисхах значений

        for (size_t n = 0; n < _line.size(); n++)
        {
            auto entry = &_line.at(n);
            auto key = entry->key;
            auto value = root[key].as<char *>();
            if (value)
            {
                entry->value = value;
            }
        }
        // мы будем рисовать уже готовый набор - с такой скоростью смены страниц как хоитм
        // так как разорвали сбор и отображение данных

        draw();
    }

    void draw()
    {
        if (millis() < (_lastResfresh + PAGE_UPDATE_ms))
            return;

        size_t page_num = _line.size() / LINE_NUMS;

        u8g2.clearDisplay();

        for (size_t n = _page_n * LINE_NUMS; n < (_page_n + 1) * LINE_NUMS; n++)
        {
            if (++_line_n >= LINE_NUMS || (n > _line.size())
            {
                _line_n = 0;
                _page_n++;
                break;
            }        
           auto entry = _line.at(n);
        
            int y = CHAR_HEIGHT * (_line_n + 1);
            u8g2.drawStr(0, y, entry.render.c_str());
            u8g2.drawStr(_max_key_width, y, entry.value.c_str());
            Serial.printf("%s%s\r\n", entry.render.c_str(), entry.value.c_str());
        }
        _lastResfresh = millis();
        if (_page_n >= page_num)
        {
            _page_n = 0;
        }
        u8g2.sendBuffer();
    }
}
