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
   public:
    TextPosition pos{0, 0};

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

    size_t printTo(Print &p) const {
        return p.printf("(%d, %d)", pos.row, pos.col);
    }
};