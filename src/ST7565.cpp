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
#define PAGE_CHANGE_ms 1000
#define DESC_LEN 8
#define LINE_LEN 24
#define PARAM_LEN (LINE_LEN - DESC_LEN)

static constexpr uint8_t CHAR_HEIGHT = 10;
//

namespace ST7565 {
struct Line {
    // Ключ
    String key;
    // Описание
    String desc;
    // Значение
    String value;
    // Длина в пикселях
    uint8_t desc_width;
    // значение изменилось
    bool updated;

    Line(const String key, const String desc, const String value) : key{key}, desc{desc}, value{value} {
        desc_width = calcWidth(desc.c_str());
    }

    bool update(const char *str) {
        if (!value.equals(str)) {
            value = str;
            updated = true;
            return true;
        }
        return false;
    }
    
};

std::vector<Line> _line;
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
void init() {
    u8g2.begin();
    u8g2.setContrast(30);
    u8g2.setFont(u8g2_font_ncenB08_tr);  // choose a suitable font

    u8g2.clearBuffer();
}

void clear() {
}

char *renderKey(char *buf, size_t buf_size, const char *str) {
    snprintf(buf, buf_size, "%s: ", str);
    return buf;
}

size_t calcWidth(const char *str) {
    return u8g2.getUTF8Width(str);
}

size_t drawStr(const char *str, uint8_t x, uint8_t y) {
    return u8g2.drawStr(x, y, str);
}

size_t drawLine(uint8_t x, uint8_t y, const char *key, const char *value) {
    char buf[LINE_LEN + 1];
    renderKey(buf, sizeof(key), key);
    size_t value_x = drawStr(buf, x, y);
    return drawStr(value, value_x + x, y);
}

Line *findKey(const char *key) {
    for (size_t i = 0; i < _line.size(); i++) {
        auto entry = &_line.at(i);
        if (entry->key.equalsIgnoreCase(key))
            return entry;
    }
    return nullptr;
}

void calcMaxWidth() {
    _max_desc_width = 0;
    for (auto entry : _line) {
        if (_max_desc_width < entry.desc_width)
            _max_desc_width = entry.desc_width;
    }
}

void fillLine(String json) {
    StaticJsonBuffer<512> doc;
    JsonObject &root = doc.parseObject(json);

    // Обновим параметры
    for (JsonObject::iterator it = root.begin(); it != root.end(); ++it) {
        auto key = (*it).key;
        auto value = (*it).value.as<char *>();
        auto entry = findKey(key);
        if (!entry) {
            char buf[LINE_LEN + 1];
            renderKey(buf, sizeof(buf), key);
            _line.push_back(Line(key, buf, value));
        } else {
            entry->update(value);
        }
    }

    calcMaxWidth();
}

// У нас сразу есть даные чтобы выводить их
// {"ip":"192.168.25.169","time":"23.01.22 01:18:44","weekday":"1","timenow":"01:18","upt":"00:00:18","any109":"0.00","any248":"0.00","any230":"0.00"}
void show(const String &json) {
    Serial.println(json);

    if (!_init) {
        init();
        _init = true;
    }

    fillLine(json);

    draw();
}

void draw() {
    if (_lastResfresh && (millis() < (_lastResfresh + PAGE_UPDATE_ms)))
        return;

    size_t page_cnt = _line.size() / LINE_NUMS;

    if (millis() > (_nextPage + PAGE_CHANGE_ms))
        if (++_page_n >= page_cnt) {
            _page_n = 0;
            u8g2.clearDisplay();
            _nextPage = millis();
            _fullDraw = true;
        }

    size_t line_offset = _page_n * LINE_NUMS;

    size_t page_line = 0;
    for (size_t n = line_offset; n < (line_offset + LINE_NUMS) && (n < _line.size()); n++) {
        auto entry = _line.at(n);
        int y = CHAR_HEIGHT * page_line++;
        if (_fullDraw) {
            u8g2.drawStr(0, y, entry.desc.c_str());
            Serial.printf("desc: %s\r\n", entry.desc.c_str());
        }
        if (_fullDraw || entry.updated) {
            u8g2.drawStr(entry.desc_width, y, entry.value.c_str());
            entry.updated = false;
            Serial.printf("value: %s\r\n", entry.value.c_str());
        }
    }

    u8g2.sendBuffer();

    _lastResfresh = millis();
}
}  // namespace ST7565
