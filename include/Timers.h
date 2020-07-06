#include <Arduino.h>

class Timer {
   public:
    Timer(const char* name, unsigned long time);
    void setTime(unsigned long time);
    const char* name() const;
    bool tick();
    void stop();

   private:
    void onTimer();

   private:
    bool _enabled;
    unsigned long _time;
    char _name[16];
};

namespace Timers {
void loop();
Timer* add(const String& name, unsigned long time);
void erase(const String& name);
}