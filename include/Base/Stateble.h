
#pragma once

#include <Arduino.h>

class Stateble {
   public:
    Stateble(int initial);
    void setState(int value);
    void setStateAsBool(bool value);
    void toogleState();
    int getState() const;
    void updateState() {
        onStateChange();
    }
    virtual void onInit(){};
    virtual void onStateChange(){};

   private:
    int _state;
};
