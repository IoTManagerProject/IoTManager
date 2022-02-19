#pragma once

#include <Print.h>
#include <stdint.h>

#define DEBUG_DISPLAY

#define DEFAULT_PAGE_UPDATE_ms 1000
#define DEFAULT_PAGE_TIME_ms 5000

#ifndef DEBUG_DISPLAY
#define D_LOG(fmt, ...) \
    do {                \
        (void)0;        \
    } while (0)
#else
#define D_LOG(fmt, ...) Serial.printf((PGM_P)PSTR(fmt), ##__VA_ARGS__)
#endif

static const char SETTINGS_FILE[] = "/display.json";
static const char SETTINGS_DEFAULT[] = "{\"type\": \"ST7556\", \"connection\": \"\", \"page_count\": 0}";

enum rotation_t : uint8_t {
    ROTATION_NONE,
    ROTATION_90,
    ROTATION_180,
    ROTATION_270
};

uint8_t parse_contrast(const char* str) {
    uint8_t val = 30;
    if (strlen(str)) {
        int val = atoi(str);
        if (val > 100) val = 100;
    }
    return val;
}

rotation_t parse_rotation(const char* str) {
    if (strlen(str)) {
        int val = atoi(str);
        if (val > 0 && val <= 90) return ROTATION_90;
        if (val > 90 && val <= 180) return ROTATION_180;
        if (val > 180 && val <= 270) return ROTATION_270;
    }
    return ROTATION_NONE;
}

struct DisplayPage {
    String key;
    uint16_t time;
    rotation_t rotation;
    String font;
    String format;
    String valign;

    DisplayPage(
        const String& key,
        uint16_t time,
        rotation_t rotation,
        const String& font) : key{key}, time{time}, rotation{rotation}, font{font} {}

    void load(const JsonObject& obj) {
        if (obj["time"].success()) time = obj["time"].as<uint16_t>();
        if (obj["rotation"].success()) rotation = parse_rotation(obj["rotation"].as<char*>());
        if (obj["font"].success()) font = obj["font"].as<char*>();
        valign = obj["valign"].success() ? obj["valign"].as<char*>() : "";
        format = obj["format"].success() ? obj["format"].as<char*>() : "%s: %s";
    }
};

enum position_t {
    POS_AUTO,
    POS_ABSOLUTE,
    POS_RELATIVE,
    POS_TEXT
};

struct RelativePosition {
    float x;
    float y;
};

struct TextPosition {
    uint8_t row;
    uint8_t col;
};

struct Point {
    uint16_t x;
    uint16_t y;

    Point() : Point(0, 0) {}

    Point(uint16_t x, uint16_t y) : x{x}, y{y} {}

    Point(const Point& rhv) : Point(rhv.x, rhv.y) {}
};

struct Position {
    position_t type;
    union {
        Point abs;
        RelativePosition rel;
        TextPosition text;
    };

    Position() : type{POS_AUTO} {}

    Position(const Point& pos) : type{POS_ABSOLUTE} {
        abs.x = pos.x;
        abs.y = pos.y;
    }

    Position(const RelativePosition& pos) : type{POS_RELATIVE} {
        rel.x = pos.x;
        rel.y = pos.y;
    }

    Position(const TextPosition& pos) : type{POS_TEXT} {
        text.col = pos.col;
        text.row = pos.row;
    }

    Position(const Position& rhv) : type{rhv.type} {
        switch (type) {
            case POS_ABSOLUTE:
                abs = rhv.abs;
            case POS_RELATIVE:
                rel = rhv.rel;
            case POS_TEXT:
                text = rhv.text;
            default:
                break;
        }
    }
};

class Cursor : public Printable {
   private:
    Point _size;

   public:
    TextPosition pos{0, 0};
    Point abs{0, 0};
    Point chr;
    Cursor(){};

    Cursor(const Point& size, const Point& chr) : _size{size}, chr{chr} {
        D_LOG("w: %d, h: %d, ch: %d(%d)\r\n", _size.x, _size.y, chr.x, chr.y);
    }

    void reset() {
        pos.col = 0;
        pos.row = 0;
        abs.x = 0;
        abs.y = 0;
    }

    void lineFeed() {
        pos.col = 0;
        pos.row++;
        abs.x = 0;
        abs.y += chr.y;
    }

    void moveX(uint8_t x) {
        abs.x += x;
        pos.col = abs.x / chr.x;
    }

    void moveY(uint8_t y) {
        abs.y += y;
    }

    void moveXY(uint8_t x, uint8_t y) {
        moveX(x);
        moveY(y);
    }

    void moveCarret(uint8_t col) {
        pos.col += col;
        moveX(col * chr.x);
    }

    bool isEndOfPage(uint8_t rows = 1) {
        return (abs.y + (rows * chr.y)) > _size.y;
    }

    bool isEndOfLine(uint8_t cols = 1) {
        return (abs.x + (cols * chr.x)) > _size.x;
    }

    size_t printTo(Print& p) const {
        return p.printf("(c:%d, r:%d x:%d, y:%d)", pos.col, pos.row, abs.x, abs.y);
    }
};