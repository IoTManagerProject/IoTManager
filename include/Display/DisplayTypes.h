#pragma once

#include <stdint.h>
#include <Print.h>

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

struct AbsolutePosition {
    uint16_t x;
    uint16_t y;
    
    AbsolutePosition(): AbsolutePosition(0,0) {}
    
    AbsolutePosition(uint16_t x, uint16_t y): x{x}, y{y} {}
    
    AbsolutePosition(const AbsolutePosition& rhv): AbsolutePosition(rhv.x, rhv.y) {}
};

struct Position {
    position_t type;
    union {
        AbsolutePosition abs;
        RelativePosition rel;
        TextPosition text;
    };

    Position() : type{POS_AUTO} {}

    Position(const AbsolutePosition& pos) : type{POS_ABSOLUTE} {
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
    AbsolutePosition _max;
   public:   
    TextPosition pos{0, 0};
    Cursor() {};

    Cursor(const AbsolutePosition& max): _max{max} {};

    void reset() {
        pos.col = 0;
        pos.row = 0;
    }

    void nextRow() {
        pos.row++;
        pos.col = 0;
    }

    void nextCol(uint8_t col) {
        pos.col += col;
    }

    bool isEOR(int8_t line_height, uint8_t rows = 1) {
        return ((pos.row + rows) * line_height) > _max.y;
    }

    bool isEOL(uint8_t char_width, uint8_t cols = 1) {
        return ((pos.col + cols) * char_width) > _max.x;
    }

    size_t printTo(Print &p) const {
        return p.printf("(x:%d, y:%d)", pos.col, pos.row);
    }
};