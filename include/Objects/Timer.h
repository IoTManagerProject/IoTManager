#pragma once

#include <Arduino.h>

class Timer {
   public:
    Timer(const char* name, unsigned long time);
    ~Timer();

    void setTime(unsigned long time);
    const char* name() const;
    bool tick();
    void stop();

   private:
    void onTimer();

   private:
    bool _enabled;
    unsigned long _time;
    char* _name;
};
