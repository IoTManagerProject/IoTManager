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

#ifdef DISPLAY_I2C
#include <Wire.h>
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
#else
U8G2_ST7565_ERC12864_F_4W_SW_SPI u8g2(U8G2_R0, D6, D7, D8, D4, D3);
#endif

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
        _obj->enableUTF8Print();
        // width: 128, height: 64, char_height: 11, lines: 5
        // 64 - 55 = 9 / 5 = 2
        Serial.printf("width: %d, height: %d, char_height: %d(%d), lines: %d\r\n", getWidth(), getHeight(), getMaxCharHeight(), getLineYSpacer(), getLines());
        clear();
    }

    uint8_t drawAtLine(uint8_t x, uint8_t n, const String &str, bool inFrame = false) {
        int y = getLineY(n);
        // x, y нижний левой
        uint8_t width = _obj->drawStr(x, y, str.c_str());
        Serial.printf("x:%d y:%d w:%d\t", x, y, width);
        if (inFrame) {
            uint8_t spacer = getLineYSpacer();
            // x, y верхней  длина, высота
            _obj->drawFrame(x - spacer, y - (getMaxCharHeight() + spacer), width + spacer, getMaxCharHeight() + spacer);
        }
        return width;
    }

    uint8_t getLineY(uint8_t n) {
        return (n + 1) * getLineHeight();
    }

    uint8_t getLineHeight() {
        return getMaxCharHeight() + getLineYSpacer();
    }

    int getLineYSpacer() {
        return (getHeight() - (getLines() * getMaxCharHeight())) / getLines();
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
        return _obj->getMaxCharHeight();
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

struct ParamPropeties {
    // рамка
    bool frame[false];
};

struct Param {
    // Ключ
    const String key;
    // Описание
    String descr;
    // Значение
    String value;
    // значение изменилось
    bool updated;
    // группа
    uint8_t group;
    
    ParamPropeties props;

    Param(const String &key, const String &value = emptyString, const String &descr = emptyString) : key{key}, group{0} {
        setValue(value.c_str());
    }

    bool isValid() {
        return !descr.isEmpty();
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

    void draw(Display *obj, uint8_t line) {
        Serial.printf("%d %s '%s%s'\r\n", line, key.c_str(), descr.c_str(), value.c_str());
        size_t width = obj->drawAtLine(0, line, descr.c_str());
        obj->drawAtLine(width, line, value.c_str(), props.frame);
    }
};

class ParamCollection {
    std::vector<Param> _item;

   public:
    void load(String dataJson, String eventJson) {
        Serial.print(dataJson);
        Serial.print(dataJson);
        StaticJsonBuffer<512> eventDoc;
        JsonObject &event = eventDoc.parseObject(eventJson);
        StaticJsonBuffer<512> dataDoc;
        JsonObject &data = dataDoc.parseObject(dataJson);
        auto key = event["key"].as<char *>();
        auto valueKey = event["val"].as<char *>();
        auto value = data[valueKey].as<char *>();
        auto descr = event["descr"].as<char *>();
        auto entry = find(key);
        if (!entry) {
            _item.push_back(Param(key, value));
        } else {
            entry->setValue(value);
            entry->setDescr(descr);
        }
    }

    Param *find(const char *key) {
        Param *res = nullptr;
        for (size_t i = 0; i < _item.size(); i++) {
            if (_item.at(i).key.equalsIgnoreCase(key)) {
                res = &_item.at(i);
                break;
            }
        }
        return res;
    }

    // n - номер по порядку
    Param *get(int n) {
        for (size_t i = 0; i < _item.size(); i++)
            if (_item.at(i).isValid())
                if (!(n--)) return &_item.at(i);
        return nullptr;
    }

    size_t count() {
        size_t res = 0;
        for (auto entry : _item) res += entry.isValid();
        return res;
    }

    size_t max_group() {
        size_t res = 0;
        for (auto entry : _item)
            if (res < entry.group) res = entry.group;
        return res;
    }
};

namespace ST7565 {
// текущая
size_t _page_n{0};
Display *_display{nullptr};
ParamCollection *_param{nullptr};
unsigned long _pageChange{0};
bool _pageChanged{true};
uint8_t _max_descr_width{0};

typedef std::vector<Param *> Line;

struct Page {
    std::vector<Line *> line;
};

uint8_t getPageCount(ParamCollection *param, uint8_t linesPerPage) {
    size_t res = 0;
    size_t totalLines = param->count();
    if (totalLines && linesPerPage) {
        res = totalLines / linesPerPage;
        if (totalLines % linesPerPage) res++;
    }
    return res;
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

void show(const String &data, const String &event) {
    if (!_display)
        _display = new Display(&u8g2);

    if (!_param)
        _param = new ParamCollection();

    _param->load(data, event);

    size_t param_cnt = _param->count();

    if (!param_cnt) return;

    if (_pageChanged) _display->clear();

    size_t page_cnt = getPageCount(_param, _display->getLines());

    if (_display->isNeedsRefresh() || _pageChanged) {
        Serial.printf("page: %d/%d\r\n", _page_n + 1, page_cnt);
        draw(_page_n);
    }

    if (millis() >= (_pageChange + PAGE_CHANGE_ms)) {
        if (++_page_n >= page_cnt) _page_n = 0;
        _pageChanged = true;
        _pageChange = millis();
    } else {
        _pageChanged = false;
    }
}

void draw(uint8_t page) {
    size_t linesPerPage = _display->getLines();
    size_t line_first = _page_n * linesPerPage;
    size_t line_last = line_first + linesPerPage - 1;

    _display->startRefresh();

    size_t lineOfPage = 0;
    for (size_t n = line_first; n <= line_last; n++) {
        auto entry = _param->get(n);
        if (entry) {
            entry->draw(_display, lineOfPage);
            lineOfPage++;
        } else {
            break;
        }
    }
    _display->endRefresh();
}

}  // namespace ST7565
