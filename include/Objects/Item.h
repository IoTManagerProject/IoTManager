
#pragma once
#include <Arduino.h>

class Nameble {
   public:
    Nameble(const char* name) {
        strlcpy(_name, name, sizeof(_name));
    }
    const char* name() {
        return _name;
    }
    char _name[33];
};

class Stateble {
   public:
    Stateble(int state) {
        _state = state;
    }

    bool setBool(bool value) {
        _state = value;
        onStateChange();
        return _state;
    }

    int setInt(int value) {
        _state = value;
        onStateChange();
        return _state;
    }

    int state() const {
        return _state;
    }

    virtual void onStateChange() const {};

   private:
    int _state;
};

class Assignable {
   public:
    Assignable(const char* assign) {
        strlcpy(_assign, assign, sizeof(_assign));
    }
    const char* assign() {
        return _assign;
    }

    uint8_t getPin() const {
        return String(_assign).toInt();
    }

    char _assign[8];
};