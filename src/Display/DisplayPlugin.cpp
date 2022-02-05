#include "Display/DisplayPlugin.h"
#include "Display/DisplayTypes.h"
#include "Display/DisplayFactory.h"
#include "Display/DisplaySettings.h"


#include <vector>

#define STRHELPER(x) #x
#define TO_STRING_AUX(...) "" #__VA_ARGS__
#define TO_STRING(x) TO_STRING_AUX(x)

// главное чтобы не меньше была
#define LINE_LEN 24


class Display {
   private:
    unsigned long _lastResfresh{0};
    Cursor _cursor;
    U8G2 *_obj{nullptr};

   public:
    Display(U8G2 *obj) : _obj{obj} {
        init();
    }

    void init() {
        _obj->begin();
        _obj->setContrast(30);
        _obj->setFont(u8g2_font_ncenB08_tr);
        _obj->enableUTF8Print();
        _cursor = Cursor({getHeight(), getWidth()});
        D_LOG("w: %d, h: %d, ch: %d(%d), l: %d\r\n", getWidth(), getHeight(), getMaxCharHeight(), getYSpacer(), getLines());
        clear();
    }

    void getPosition(const TextPosition &a, AbsolutePosition &b) {
        b.x = a.col * getMaxCharWidth();
        b.y = (a.row + 1) * getLineHeight();
    }

    void getPosition(const RelativePosition &a, AbsolutePosition &b) {
        b.x = getHeight() * a.x;
        b.y = getWidth() * a.y;
    }

    void getPosition(const AbsolutePosition &a, TextPosition &b) {
        b.row = a.y / getLineHeight();
        b.col = a.x / getMaxCharWidth();
    }

    void getPosition(const RelativePosition &a, TextPosition &b) {
        AbsolutePosition tmp;
        getPosition(a, tmp);
        getPosition(tmp, b);
    }



    uint8_t draw(const RelativePosition &pos, const String &str) {
        AbsolutePosition tmp;
        getPosition(pos, tmp);
        return draw(tmp, str);
    }

    uint8_t draw(TextPosition &pos, const String &str) {
        AbsolutePosition tmp;
        getPosition(pos, tmp);
        return draw(tmp, str);
    }

    uint8_t draw(const AbsolutePosition &pos, const String &str) {
        return _obj->drawStr(pos.x, pos.y, str.c_str());
    }
    
    uint8_t println(const String &str, bool frame = false) {
        AbsolutePosition pos;
        getPosition(_cursor.pos, pos);
        auto res = draw(pos, str.c_str());
        _cursor.nextRow();
        Serial.print(_cursor);
        return res;
    }


    bool isEOR(uint8_t rows = 1) {
         return _cursor.isEOL( getLineHeight(), rows);
    }

    bool isEOL(uint8_t cols = 1) {
        return _cursor.isEOL( getMaxCharWidth(), cols);
    }

    uint8_t print(const String &str, bool frame = false) {
        if (isEOL(str.length())) _cursor.nextRow();
        AbsolutePosition pos;
        getPosition(_cursor.pos, pos);
        uint8_t width = draw(pos, str.c_str());
        _cursor.nextCol(width / getMaxCharWidth());
        Serial.print(_cursor);
        return width;
    }

    uint8_t print(uint8_t x, uint8_t n, const String &str, bool frame = false) {
        int y = getLineY(n);
        // x, y нижний левой
        uint8_t width = _obj->drawStr(x, y, str.c_str());
        D_LOG("x:%d y:%d w:%d", x, y, width);
        if (frame) {
            x -= getXSpacer();
            y -= getMaxCharHeight();
            width += getXSpacer() * 2;
            int height = getMaxCharHeight() + getYSpacer() * 2;

            // x, y верхней левой блин. длина, высота
            _obj->drawFrame(x, y, width, height);

            D_LOG("[x:%d y:%d w:%d h:%d]", x, y, width, height);
        }
        return width;
    }

    uint8_t getLineY(uint8_t n) {
        return (n + 1) * getLineHeight();
    }

    uint8_t getLineHeight() {
        return getMaxCharHeight() + getYSpacer();
    }

    int getXSpacer() {
        int res = getWidth() / 100;
        if (!res) res = 1;
        return res;
    }

    int getYSpacer() {
        int res = (getHeight() - (getLines() * getMaxCharHeight())) / getLines();
        if (!res) res = 1;
        return res;
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

    uint8_t getMaxCharWidth() {
        return _obj->getMaxCharWidth();
    }

    void clear() {
        _obj->clearDisplay();
        _cursor.reset();
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
    Position position;

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
    }

    void draw(Display *obj) {
        auto type = position.type;
        switch (type) {
            case POS_AUTO: {
                D_LOG("AUTO %s '%s%s'\r\n", key.c_str(), descr.c_str(), value.c_str());
                obj->print(descr.c_str());
                obj->println(value.c_str(), false);
            }
            case POS_ABSOLUTE: {
                auto pos = position.abs;
                D_LOG("ABS(%d, %d) %s %s'\r\n", pos.x, pos.y, key.c_str(), value.c_str());
                obj->draw(pos, value);
            }
            case POS_RELATIVE: {
                auto pos = position.rel;
                D_LOG("REL(%2.2f, %2.2f) %s %s'\r\n", pos.x, pos.y, key.c_str(), value.c_str());
                obj->draw(pos, value);
            }
            case POS_TEXT: {
                auto pos = position.text;
                D_LOG("TXT(%d, %d) %s %s'\r\n", pos.col, pos.row, key.c_str(), value.c_str());
                obj->draw(pos, value);
            }
            default:
                D_LOG("unhadled: %d", type);
        }
    }
};

class ParamCollection {
    std::vector<Param> _item;

   public:
    void load(String dataJson, String eventJson) {
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

    Param *get(int n) {
        return &_item.at(n);
    }

    size_t count() {
        return _item.size();
    }

    // n - номер по порядку параметра
    Param *getValid(int n) {
        for (size_t i = 0; i < _item.size(); i++)
            if (_item.at(i).isValid())
                if (!(n--)) return &_item.at(i);
        return nullptr;
    }

    size_t getVaildCount() {
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

namespace DisplayPlugin {

Display *_display{nullptr};
ParamCollection *_param{nullptr};
DisplaySettings _context;
bool _inited{false};

// последний отображенный
uint8_t _n{0};
unsigned long _lastPageChange{0};
bool _pageChanged{true};
uint8_t _max_descr_width{0};
typedef std::vector<Param *> Line;
// текущая
size_t _page_n{0};
struct Page {
    std::vector<Line *> line;
};

uint8_t calcPageCount(ParamCollection *param, uint8_t linesPerPage) {
    size_t res = 0;
    size_t totalLines = param->count();
    if (totalLines && linesPerPage) {
        res = totalLines / linesPerPage;
        if (totalLines % linesPerPage) res++;
    }
    return res;
}

uint8_t getPageCount() {
    return _context.isAutoPage() ? calcPageCount(_param, _display->getLines()) : _context.getPageCount();
}

// выводит на страницу параметры начиная c [n]
// возвращает [n] последнего уместившегося
uint8_t draw(Display *display, ParamCollection *param, uint8_t n) {
    uint8_t i;
    for (i = n; i < param->count(); i++) {
        auto entry = param->get(i);
        display->print(entry->descr);
        display->println(entry->value);
        if (display->isEOR()) break;
    }
    return i;
}

void draw(Display *display, uint8_t page) {
    size_t linesPerPage = display->getLines();
    size_t line_first = _page_n * linesPerPage;
    size_t line_last = line_first + linesPerPage - 1;

    display->startRefresh();

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
    display->endRefresh();
}
//{"key":"any248","addr":"","int":"10","c":"0","k":"60","val":"time","type":"ST7565","descr":"Hum"}
// {"ip":"192.168.25.169","time":"23.01.22 01:18:44","weekday":"1","timenow":"01:18","upt":"00:00:18","any109":"0.00","any248":"0.00","any230":"0.00"}

void show(Display *display, ParamCollection *param) {
    size_t param_count = param->count();

    if (!param_count) return;

    if (_pageChanged) display->clear();

    if (display->isNeedsRefresh() || _pageChanged) {
        D_LOG("[Display] n: %d/%d\r\n", _n, param_count);
        auto res = draw(display, param, _n);
        if (_pageChanged) _n = res;
    }

    _pageChanged = false;

    if (millis() >= (_lastPageChange + PAGE_CHANGE_ms)) {
        if (_n >= param_count) _n = 0;
        _pageChanged = true;
        _lastPageChange = millis();
    }
}

void show(const String &data, const String &event) {
    if(!_inited) {
        _context.init();
        _param = new ParamCollection();
        _display = new Display(DisplayFactory().createInstance(_context.getType(), _context.getConnection()));        
        _inited = true;
    }

    _param->load(data, event);

    show(_display, _param);
}

}  // namespace ST7565
