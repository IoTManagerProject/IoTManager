#pragma once

#include "Module/EditLine.h"

#define A_NORMAL 0x0000     // normal
#define A_UNDERLINE 0x0001  // underline
#define A_REVERSE 0x0002    // reverse
#define A_BLINK 0x0004      // blink
#define A_BOLD 0x0008       // bold
#define A_DIM 0x0010        // dim
#define A_STANDOUT A_BOLD   // standout (same as bold)

#define F_BLACK 0x0100    // foreground black
#define F_RED 0x0200      // foreground red
#define F_GREEN 0x0300    // foreground green
#define F_BROWN 0x0400    // foreground brown
#define F_BLUE 0x0500     // foreground blue
#define F_MAGENTA 0x0600  // foreground magenta
#define F_CYAN 0x0700     // foreground cyan
#define F_WHITE 0x0800    // foreground white
#define F_YELLOW F_BROWN  // some terminals show brown as yellow (with A_BOLD)
#define F_COLOR 0x0F00    // foreground mask

#define B_BLACK 0x1000    // background black
#define B_RED 0x2000      // background red
#define B_GREEN 0x3000    // background green
#define B_BROWN 0x4000    // background brown
#define B_BLUE 0x5000     // background blue
#define B_MAGENTA 0x6000  // background magenta
#define B_CYAN 0x7000     // background cyan
#define B_WHITE 0x8000    // background white
#define B_YELLOW B_BROWN  // some terminals show brown as yellow (with A_BOLD)
#define B_COLOR 0xF000    // background mask

#define CHAR_NULL 0x00
#define CHAR_BEL 0x07
#define CHAR_BS 0x08
#define CHAR_SPACE 0x20
#define CHAR_TAB 0x09
#define CHAR_LF 0x0a
#define CHAR_CR 0x0d
#define CHR_ZERO 0x30

#define KEY_DEL 0x7f
#define KEY_DOWN 0x80
#define KEY_UP 0x81
#define KEY_LEFT 0x82
#define KEY_RIGHT 0x83
#define KEY_HOME 0x84
#define KEY_INS 0x86
#define KEY_PAGE_DOWN 0x87
#define KEY_PAGE_UP 0x88
#define KEY_END 0x89
#define CHAR_LT 0x8b
#define CHAR_CSI 0x9b
#define CHAR_ESC 0x1b
#define CHAR_BIN 0xFF

#define ESC_CURSOR_HOME "\x1b[H"
#define ESC_SAVE_CURSOR "\x1b[s"
#define ESC_UNSAVE_CURSOR "\x1b[u"
#define ESC_SAVE_CURSOR_AND_ATTRS "\x1b[7"
#define ESC_RESTORE_CURSOR_AND_ATTRS "\x1b[8"

#define ESC_CLEAR "\x1b[2J"
#define ESC_CLEAR_BOTTOM "\x1b[J"
#define ESC_CLEAR_EOL "\x1b[0K"

#define ESC_CURSOR_UP "\x1b[1A"
#define ESC_CURSOR_DOWN "\x1b[1B"
#define ESC_CURSOR_FORWARD "\x1b[1C"
#define ESC_CURSOR_BACKWARD "\x1b[1D"

#define SEQ_CSI PSTR("\033[")       // code introducer
#define SEQ_LOAD_G1 PSTR("\033)0")  // load G1 character set
#define SEQ_CLEAR PSTR("\033[2J")   // clear screen
#define SEQ_ATTRSET PSTR("\033[0")  // set attributes, e.g. "\033[0;7;1m"

#define SEQ_ATTRSET_BOLD PSTR(";1")       // bold
#define SEQ_ATTRSET_DIM PSTR(";2")        // dim
#define SEQ_ATTRSET_FCOLOR PSTR(";3")     // forground color
#define SEQ_ATTRSET_UNDERLINE PSTR(";4")  // underline
#define SEQ_ATTRSET_BCOLOR PSTR(";4")     // background color
#define SEQ_ATTRSET_BLINK PSTR(";5")      // blink
#define SEQ_ATTRSET_REVERSE PSTR(";7")    // reverse

enum TerminalEventEnum {
    EVENT_OPEN,
    EVENT_CLOSE,
    EVENT_TAB
};

enum SpecialKeyEnum { SPEC_KEY_UP,
                      SPEC_KEY_TAB,
                      SPEC_KEY_ENTER,
                      SPEC_KEY_ESC };

typedef std::function<bool(SpecialKeyEnum key)> SpecialKeyPressedEvent;

typedef std::function<void(TerminalEventEnum, Stream *)> TerminalEventHandler;

typedef std::function<void(const char *)> TerminalInputEventHandler;

enum EOLType_t { CRLF,
                 LFCR,
                 LF,
                 CR };

enum State { ST_INACTIVE,
             ST_NORMAL,
             ST_ESC_SEQ,
             ST_CTRL_SEQ };

class Terminal : public Print {
   public:
    Terminal(Stream *stream = nullptr);
    ~Terminal();
    void setStream(Stream *stream);
    void setEOL(EOLType_t code);
    void enableControlCodes(bool enabled = true);
    void enableEcho(bool enabled = true);
    void enableColors(bool enabled = true);
    void setOnEvent(TerminalEventHandler);
    void setOnSpecKeyPress(SpecialKeyPressedEvent);
    void setOnReadLine(TerminalInputEventHandler);

    bool setLine(const uint8_t *bytes, size_t size);
    CharBuffer &getLine();

    void backsp();
    void clear();
    void clear_line();
    size_t println(const char *str);
    size_t println(void);
    size_t write_P(PGM_P str);
    size_t write(uint8_t c);
    size_t write(const uint8_t *buf, size_t size);
    void writeByDigit(uint8_t i);
    bool available();
    void loop();
    void start();
    void quit();
    void initscr();
    void attrset(uint16_t attr);

   private:
    void move(uint8_t y, uint8_t x);
    TerminalEventHandler eventHandler_;
    TerminalInputEventHandler inputHandler_;

    uint8_t attr = 0xff;
    uint8_t curY = 0xff;
    uint8_t curX = 0xff;

    unsigned long _lastReceived = 0;
    State state = ST_INACTIVE;
    Stream *_stream;
    EditLine _line;
    char _cc_buf[32] = {0};
    size_t _cc_pos = 0;
    bool _color = false;
    bool _controlCodes = false;
    bool _echo = false;
    EOLType_t _eol = CRLF;

    struct ControlCode {
        const char *cc;
        const char ch;
    };

    ControlCode keyMap[10] = {
        {"G", KEY_HOME},       // 71 Home key
        {"H", KEY_UP},         // 72 Up arrow
        {"I", KEY_PAGE_UP},    // 73 PageUp
        {"K", KEY_LEFT},       // 75 Left arrow
        {"M", KEY_RIGHT},      // 77 Right arrow
        {"O", KEY_END},        // 79 End key
        {"P", KEY_DOWN},       // 80 Down arrow
        {"Q", KEY_PAGE_DOWN},  // 81 PageDown
        {"R", KEY_INS},        // 82 Insert
        {"S", KEY_DEL},        // 83 Delete
    };
};
