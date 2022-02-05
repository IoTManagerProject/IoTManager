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

struct Point {
    uint16_t x;
    uint16_t y;
    
    Point(): Point(0,0) {}
    
    Point(uint16_t x, uint16_t y): x{x}, y{y} {}
    
    Point(const Point& rhv): Point(rhv.x, rhv.y) {}
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
    Cursor() {};

    Cursor(const Point& size,  const Point& chr): _size{size}, chr{chr} {
        D_LOG("w: %d, h: %d, ch: %d(%d)\r\n", _size.x, _size.y,  chr.x, chr.y);
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
        abs.x= 0;
        abs.y += chr.y;
    }

    void moveX(uint8_t x) {
        abs.x+=x;        
        pos.col = abs.x / chr.x;
    }

    void moveY(uint8_t y) {
        abs.y+=y;
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

    size_t printTo(Print &p) const {
        return p.printf("(c:%d, r:%d x:%d, y:%d)", pos.col, pos.row, abs.x, abs.y);
    }
};