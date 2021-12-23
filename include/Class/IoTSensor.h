#pragma once

#include <WString.h>

class IoTSensor {
   public:
    IoTSensor();
    ~IoTSensor();

    void loop();
    virtual void doByInterval();
    void init(unsigned long interval, String key);
    void regEvent(String value, String consoleInfo);

    unsigned long currentMillis;
    unsigned long prevMillis;
    unsigned long difference;
    unsigned long _interval;
    String _key;
};