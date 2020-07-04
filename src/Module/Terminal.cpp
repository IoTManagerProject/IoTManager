#include "Module/Terminal.h"

#include "Utils/TimeUtils.h"
#include "Utils/PrintMessage.h"

#define INPUT_MAX_LENGHT 255

static const char *MODULE = "Term";

Terminal::Terminal(Stream *stream) : _stream{stream},
                                     _line(INPUT_MAX_LENGHT),
                                     _cc_pos(0),
                                     _color(false),
                                     _controlCodes(false),
                                     _echo(false),
                                     _eol(CRLF) { state = ST_NORMAL; };

void Terminal::setStream(Stream *stream) {
    _stream = stream;
}

void Terminal::setOnReadLine(TerminalInputEventHandler h) { inputHandler_ = h; }

void Terminal::setOnEvent(TerminalEventHandler h) { eventHandler_ = h; }

bool Terminal::available() {
    return _stream != nullptr ? _stream->available() : false;
}

void Terminal::setEOL(EOLType_t eol) {
    _eol = eol;
}

void Terminal::enableEcho(bool enabled) {
    _echo = enabled;
}

void Terminal::enableColors(bool enabled) {
    _color = enabled;
}

void Terminal::enableControlCodes(bool enabled) {
    _controlCodes = enabled;
}

void Terminal::quit() {}

void Terminal::loop() {
    if (_stream == nullptr || !_stream->available()) return;

    byte moveX = 0;
    byte moveY = 0;

    char c = _stream->read();

    _lastReceived = millis();

    if (state == ST_INACTIVE) {
        // wait for CR
        if (c == CHAR_CR) {
            if (eventHandler_) {
                eventHandler_(EVENT_OPEN, _stream);
                state = ST_NORMAL;
            }
        }
        // or ignore all other
        return;
    }

    if (c == CHAR_LF || c == CHAR_NULL || c == CHAR_BIN)
        return;

    // Esc
    if (c == CHAR_ESC || c == 195) {
        state = ST_ESC_SEQ;
        _cc_pos = 0;
        for (size_t i = 0; i < 2; ++i) {
            bool timeout = false;
            while (!_stream->available() &&
                   !(timeout = millis_since(_lastReceived) > 10)) {
                delay(0);
            }
            if (timeout) {
                state = ST_NORMAL;
                break;
            }
            _lastReceived = millis();
            c = _stream->read();
            _cc_buf[_cc_pos] = c;
            if ((c == '[') || ((c >= 'A' && c <= 'Z') || c == '~')) {
                _cc_pos++;
                _cc_buf[++_cc_pos] = '\x00';
            }
        }
        uint8_t i;
        for (i = 0; i < 10; ++i) {
            if (strcmp(_cc_buf, keyMap[i].cc) == 0) {
                c = keyMap[i].ch;
                state = ST_NORMAL;
            }
        }
    }

    if (state == ST_ESC_SEQ) {
        state = ST_NORMAL;
        return;
    }

    // WHEN NORMAL
    if (state == ST_NORMAL) {
        if (c == CHAR_ESC) {
            if (!_line.available()) {
                // QUIT
                state = ST_INACTIVE;
                if (eventHandler_)
                    eventHandler_(EVENT_CLOSE, _stream);
            } else {
                // CLEAR
                _line.clear();
                if (_controlCodes) {
                    clear_line();
                } else {
                    println();
                }
            }
            return;
        }

        switch (c) {
            case CHAR_CR:
                println();
                if (inputHandler_)
                    inputHandler_(_line.c_str());
                _line.clear();
                moveY++;
                break;
            case CHAR_TAB:
                if (eventHandler_)
                    eventHandler_(EVENT_TAB, _stream);
                return;
            case KEY_LEFT:
                if (_line.prev())
                    moveX--;
                break;
            case KEY_RIGHT:
                if (_line.next())
                    moveX++;
                break;
            case KEY_HOME:
                moveX = -1 * _line.home();
                break;
            case KEY_END:
                moveX = _line.end();
                break;
            case CHAR_BS:
            case KEY_DEL:
                if (_line.backspace()) {
                    backsp();
                    moveX--;
                }
                break;
            default:
                // printable ascii 7bit or printable 8bit ISO8859
                if ((c & '\x7F') >= 32 && (c & '\x7F') < 127)
                    if (_line.write(c)) {
                        if (_echo) write(c);
                        moveX++;
                    }
                break;
        }

        // if (controlCodesEnabled)
        // move(startY + moveY, startX + moveX);
    }
}

bool Terminal::setLine(const uint8_t *ptr, size_t size) {
    _line.clear();
    if (_line.write(ptr, size))
        print(_line.c_str());
    return true;
}

CharBuffer &Terminal::getLine() { return _line; }

void Terminal::start() {
    if (_controlCodes) initscr();
    println();
}

void Terminal::initscr() {
    write_P(SEQ_LOAD_G1);
    attrset(A_NORMAL);
    move(0, 0);
    clear();
}

void Terminal::attrset(const uint16_t attr) {
    uint8_t i;

    if (attr != this->attr) {
        this->write_P(SEQ_ATTRSET);

        i = (attr & F_COLOR) >> 8;

        if (i >= 1 && i <= 8) {
            this->write_P(SEQ_ATTRSET_FCOLOR);
            this->write(i - 1 + '0');
        }

        i = (attr & B_COLOR) >> 12;

        if (i >= 1 && i <= 8) {
            this->write_P(SEQ_ATTRSET_BCOLOR);
            this->write(i - 1 + '0');
        }

        if (attr & A_REVERSE)
            this->write_P(SEQ_ATTRSET_REVERSE);
        if (attr & A_UNDERLINE)
            this->write_P(SEQ_ATTRSET_UNDERLINE);
        if (attr & A_BLINK)
            this->write_P(SEQ_ATTRSET_BLINK);
        if (attr & A_BOLD)
            this->write_P(SEQ_ATTRSET_BOLD);
        if (attr & A_DIM)
            this->write_P(SEQ_ATTRSET_DIM);
        this->write('m');
        this->attr = attr;
    }
}

void Terminal::clear() { write_P(SEQ_CLEAR); }

void Terminal::clear_line() {
    write(CHAR_CR);
    write_P(ESC_CLEAR_EOL);
}

void Terminal::move(uint8_t y, uint8_t x) {
    write_P(SEQ_CSI);
    writeByDigit(y + 1);
    write(';');
    writeByDigit(x + 1);
    write('H');
    curY = y;
    curX = x;
}

void Terminal::writeByDigit(uint8_t i) {
    uint8_t ii;
    if (i >= 10) {
        if (i >= 100) {
            ii = i / 100;
            write(ii + '0');
            i -= 100 * ii;
        }
        ii = i / 10;
        write(ii + '0');
        i -= 10 * ii;
    }
    write(i + '0');
}

void Terminal::backsp() {
    write(CHAR_BS);
    write(CHAR_SPACE);
    write(CHAR_BS);
}

size_t Terminal::println(const char *str) {
    size_t n = print(str);
    return n += println();
}

size_t Terminal::println(void) {
    size_t n = 0;
    switch (_eol) {
        case CRLF:
            n += write(CHAR_CR);
            n += write(CHAR_LF);
            break;
        case LF:
            n += write(CHAR_LF);
            break;
        case LFCR:
            n += write(CHAR_LF);
            n += write(CHAR_CR);
            break;
        case CR:
            n += write(CHAR_CR);
            break;
    }
    return n;
}

size_t Terminal::write(uint8_t ch) {
    size_t n = 0;
    if (_stream)
        n = _stream->write(ch);
    return n;
}

size_t Terminal::write_P(PGM_P str) {
    uint8_t ch;
    size_t n = 0;
    while ((ch = pgm_read_byte(str + n)) != '\x0') {
        _stream->write(ch);
        n++;
    }
    return n;
}

size_t Terminal::write(const uint8_t *buf, size_t size) {
    size_t n = 0;
    while (size--) {
        if (_stream->write(*buf++))
            n++;
        else
            break;
    }
    return n;
}