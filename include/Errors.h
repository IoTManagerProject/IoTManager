#pragma once

#include <Arduino.h>

enum ErrorType_t {
    ET_NONE,
    ET_FUNCTION,
    ET_MODULE,
    ET_SYSTEM
};

enum ErrorLevel_t {
    EL_NONE,
    EL_INFO,
    EL_WARNING,
    EL_ERROR
};

String getErrorLevelStr(ErrorLevel_t level);

class Error : public Printable {
   public:
    static Error OK() {
        return Error();
    }

    static Error InfoMessage(const char *message) {
        return Error(EL_INFO, message);
    }

    static Error ErrorMessage(const char *message) {
        return Error(EL_ERROR, message);
    }

   public:
    Error() : _type{ET_NONE}, _level{EL_NONE} {};

    Error(const ErrorLevel_t level, const char *message) : Error(ET_FUNCTION, level, message){};

    Error(const ErrorType_t type, const ErrorLevel_t level, const char *message) : _type{type}, _level{level} {
        strncpy(_message, message, sizeof(_message));
    };

    const ErrorLevel_t level() const { return _level; }

    const ErrorType_t type() const { return _type; }

    const char *message() const { return _message; }

    operator bool() const { return _level != EL_NONE; }

    const String toString() const {
        char buf[128];
        sprintf(buf, "[%s] %s", getErrorLevelStr(_level).c_str(), _message);
        return String(buf);
    }

    virtual size_t printTo(Print &p) const {
        return p.println(toString().c_str());
    }

   private:
    char _message[128];
    ErrorType_t _type;
    ErrorLevel_t _level;
};
