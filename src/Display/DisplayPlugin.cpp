#include "Display/DisplayPlugin.h"

#include <vector>

#include "Display/DisplayFactory.h"
#include "Display/DisplaySettings.h"
#include "Display/DisplayTypes.h"

#define STRHELPER(x) #x
#define TO_STRING_AUX(...) "" #__VA_ARGS__
#define TO_STRING(x) TO_STRING_AUX(x)

// главное чтобы не меньше была
#define LINE_LEN 256

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
        _cursor = Cursor(
            {getWidth(), getHeight()},
            {getMaxCharHeight(), getLineHeight()});
        clear();
    }

    void getPosition(const TextPosition &a, Point &b) {
        b.x = a.col * _cursor.chr.x;
        b.y = (a.row + 1) * _cursor.chr.y;
    }

    void getPosition(const RelativePosition &a, Point &b) {
        b.x = getHeight() * a.x;
        b.y = getWidth() * a.y;
    }

    void getPosition(const Point &a, TextPosition &b) {
        b.row = a.y / getLineHeight();
        b.col = a.x / getMaxCharWidth();
    }

    void getPosition(const RelativePosition &a, TextPosition &b) {
        Point tmp;
        getPosition(a, tmp);
        getPosition(tmp, b);
    }

    void draw(const RelativePosition &pos, const String &str) {
        Point tmp;
        getPosition(pos, tmp);
        draw(tmp, str);
    }

    void draw(TextPosition &pos, const String &str) {
        Point tmp;
        getPosition(pos, tmp);
        draw(tmp, str);
    }

    Cursor *getCursor() {
        return &_cursor;
    }
    // print меняняю cursor
    void println(const String &str, bool frame = false) {
        print(str, frame);
        _cursor.lineFeed();
    }

    void print(const String &str, bool frame = false) {
        Serial.print(_cursor);
        // x, y нижний левой
        int width = _obj->drawStr(_cursor.abs.x, _cursor.abs.y + _cursor.chr.y, str.c_str());        
        if (frame) {
            int x = _cursor.abs.x - getXSpacer();
            int y = _cursor.abs.y - _cursor.chr.y;
            width += (getXSpacer() * 2);
            int height = _cursor.chr.y + getYSpacer() * 2;
            // x, y верхней левой. длина, высота
            _obj->drawFrame(x, y, width, height);        
            D_LOG("[x:%d y:%d w:%d h:%d]", x, y, width, height);
        }
        _cursor.moveX(width);                
    }
    
    // draw не меняет cursor
    void draw(const Point &pos, const String &str) {
        Serial.printf("(x:%d,y:%d) %s", pos.x, pos.y, str.c_str());
        _obj->drawStr(pos.x, pos.y, str.c_str());
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
        _cursor.reset();
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
    // Очищает буфер (не экран, а внутреннее представление) для последущего заполнения
    display->startRefresh();
    size_t i = 0;
    for (i = n; i < param->count(); i++) {
        auto cursor = display->getCursor();        
        auto entry = param->get(i);
        auto len = entry->value.length() + entry->descr.length();
        if (cursor->isEndOfLine(len)) cursor->lineFeed();
        display->print(entry->descr);        
        display->print(entry->value);
        if (cursor->isEndOfPage(0)) break;
    }
    // Отправит готовый буфер страницы на дисплей
    display->endRefresh();
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

    size_t last_n = _n;
    if (display->isNeedsRefresh() || _pageChanged) {
        D_LOG("[Display] n: %d/%d\r\n", _n, param_count);
        last_n = draw(display, param, _n);
    }

    if (millis() >= (_lastPageChange + PAGE_CHANGE_ms)) {
        _n = last_n;
        if (_n >= param_count) _n = 0;
        _pageChanged = true;
        _lastPageChange = millis();
    }
}

void show(const String &data, const String &event) {
    if (!_inited) {
        _context.init();
        _param = new ParamCollection();
        _display = new Display(DisplayFactory().createInstance(_context.getType(), _context.getConnection()));
        _inited = true;
    }

    _param->load(data, event);

    show(_display, _param);
}

}  // namespace DisplayPlugin
