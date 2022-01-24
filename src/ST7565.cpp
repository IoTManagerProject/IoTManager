#include "ST7565.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <U8g2lib.h>

#include <vector>

#define STRHELPER(x) #x
#define TO_STRING_AUX(...) "" #__VA_ARGS__
#define TO_STRING(x) TO_STRING_AUX(x)

#define D_LOG(fmt, ...) Serial.printf((PGM_P)PSTR(fmt), ##__VA_ARGS__)

// пока константа, но она определяет размеры выделяемого буфера и не влияет на визуализацию, главное чтобы не меньше была
#define LINE_LEN 24

U8G2_ST7565_ERC12864_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/D5, /* data=*/D7, /* cs=*/D8, /* dc=*/D2, /* reset=*/D3);

class Display {
   private:
    U8G2 *_obj{nullptr};
    unsigned long _lastResfresh{0};

   public:
    Display(U8G2 *obj) : _obj{obj} {
        init();
    }

    void init() {
        _obj->begin();
        _obj->setContrast(30);
        _obj->setFont(u8g2_font_ncenB08_tr);

        Serial.printf("width: %d, height: %d, char_height: %d, lines: %d\r\n", getWidth(), getHeight(), getMaxCharHeight(), getLines());
        clear();
    }

    void draw(uint8_t x, uint8_t y, const String &str, bool inFrame = false) {
        size_t width = _obj->drawStr(x, y, str.c_str());
        if (inFrame)
            _obj->drawFrame(x, y - _obj->getMaxCharHeight(), x + width, y);
    }

    void drawAtLine(uint8_t x, uint8_t n, const String &str, bool inFrame = false) {
        draw(x, (n + 1) * _obj->getMaxCharHeight(), str, inFrame);
    }

    uint8_t getWidth() {
        return _obj->getDisplayWidth();
    }

    uint8_t getHeight() {
        return _obj->getDisplayHeight();
    }

    uint8_t getLines() {
        return getHeight() / _obj->getMaxCharHeight();
    }

    uint8_t getMaxCharHeight() {
        return  _obj->getMaxCharHeight();
    }    
    
    void clear() {
        _obj->clearDisplay();
    }

    void startRefresh() {
        _obj->clearBuffer();
    }

    void endRefresh() {
        _obj->sendBuffer();
        _lastResfresh = millis();
    }

    bool isNeedsRefresh() {
        return (!_lastResfresh || (millis() > (_lastResfresh + DISPLAY_REFRESH_ms)));
    }
};

namespace ST7565 {
// текущая
size_t _page_n{0};
size_t _line_n{0};

Display *_display{nullptr};
uint8_t _max_descr_width{0};
uint32_t _nextPage{0};

struct Line {
    // Ключ
    const String key;
    // Описание
    String descr;
    // Значение
    String value;
    // значение изменилось
    bool updated;

    Line(const String &key, const String &value = emptyString, const String &descr = emptyString) : key{key} {
        setDescr(descr.isEmpty() ? key.c_str() : descr.c_str());
        setValue(value.c_str());
    }

    bool setDescr(const char *str) {
        char buf[LINE_LEN + 1];
        snprintf(buf, sizeof(buf), "%s: ", str);
        if (!descr.equals(buf)) {
            descr = buf;
            updated = true;
            return true;
        }
        return false;
    }

    bool setValue(const char *str) {
        if (!value.equals(str)) {
            value = str;
            updated = true;
            return true;
        }
        return false;
    }

    size_t descrWidth() {
        return calcWidth(descr.c_str());
    }

    size_t valueWidth() {
        return calcWidth(value.c_str());
    }

    static size_t calcWidth(const char *str) {
        return u8g2.getUTF8Width(str);
    }
};

std::vector<Line> _line;

Line *findKey(const char *key) {
    Line *res = nullptr;
    for (size_t i = 0; i < _line.size(); i++) {
        if (_line.at(i).key.equalsIgnoreCase(key)) {
            res = &_line.at(i);
            break;
        }
    }
    return res;
}

void calcMaxWidth() {
    _max_descr_width = 0;
    for (auto entry : _line) {
        auto width = entry.descrWidth();
        if (_max_descr_width < width)
            _max_descr_width = width;
    }
}

// Описание данных мы получаем по одному
// {"key":"any109","addr":"","int":"10","c":"0","k":"30","val":"time","type":"ST7565","descr":"Temp"}
// {"key":"any230","addr":"","int":"10","c":"0","k":"20","val":"upt","type":"ST7565","descr":"Hum"}
// {"key":"any248","addr":"","int":"10","c":"0","k":"60","val":"time","type":"ST7565","descr":"Hum"}
// WTF?! через жопу сделано
//{"key":"any248","addr":"","int":"10","c":"0","k":"50","val":"time","type":"ST7565","descr":"Tmp"}
//{"key":"any248","addr":"","int":"10","c":"0","k":"60","val":"time","type":"ST7565","descr":"Hum"}
//
// {"ip":"192.168.25.169","time":"23.01.22 01:18:44","weekday":"1","timenow":"01:18","upt":"00:00:18","any109":"0.00","any248":"0.00","any230":"0.00"}
void load( String dataJson, String eventJson) {
    StaticJsonBuffer<512> eventDoc;
    JsonObject &event = eventDoc.parseObject(eventJson);
    StaticJsonBuffer<512> dataDoc;
    JsonObject &data = dataDoc.parseObject(dataJson);
    auto key = event["key"].as<char *>();
    auto valueKey = event["val"].as<char *>();
    auto value = data[valueKey].as<char *>();
    auto entry = findKey(key);
    if (!entry) {
        _line.push_back(Line(key, value));        
    } else {
        entry->setValue(value);
    }
}


void show(const String &data, const String &event) {
    // создаем конкретный дисплей передавая его реализацию
    if (!_display)
        _display = new Display(&u8g2);
    
    load(data, event);

    draw();
}

void draw() {
    if (!_line.size()) return;
    
    if (!_display->isNeedsRefresh()) return;

    size_t page_lines = _display->getLines();

    // Количество страниц
    size_t page_cnt = _line.size() / page_lines;
    if (_line.size() % page_lines)
        page_cnt++;

    // TODO  Следующая страница в display
    if (millis() >= (_nextPage + PAGE_CHANGE_ms)) {
        if (++_page_n >= page_cnt)
            _page_n = 0;
        _display->clear();
        _nextPage = millis();
    }
    // Номер первой строки стрницы

    size_t line_first = _page_n * page_lines;
    // Номер последней строки стрницы
    size_t line_last = line_first + page_lines - 1;
    if (line_last > (_line.size() - 1))
        line_last = _line.size() - 1;

    // TODO  это задача дисплея, ему надо передать lines - что рисовать
    _display->startRefresh();
    Serial.printf("page lines: %d\r\n", page_lines);
    size_t page_line = 0;
    for (size_t n = line_first; n <= line_last; n++) {
        auto entry = &_line.at(n);
        Serial.printf("%d/%d %s: %s%s\r\n", _page_n, page_line, entry->key.c_str(), entry->descr.c_str(), entry->value.c_str());
        _display->drawAtLine(0, page_line, entry->descr);
        _display->drawAtLine(entry->descrWidth(), page_line, entry->value, true);
        entry->updated = false;
        page_line++;
    }

    _display->endRefresh();
}
}  // namespace ST7565
