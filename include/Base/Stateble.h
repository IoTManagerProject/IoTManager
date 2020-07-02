
#pragma once

#include <Arduino.h>

class Stateble {
   public:
    Stateble(int value) {
        _state = value;
    }

    void setState(int value) {
        _state = value;
        onStateChange();
    }

    void toggleState() {
        _state = !_state;
        onStateChange();
    }

    int getState() {
        return _state;
    }

    virtual void onStateChange() = 0;

   protected:
    int _state;
};