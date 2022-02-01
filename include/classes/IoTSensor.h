#pragma once

#include <WString.h>

class IoTSensor {
   public:
    IoTSensor(String parameters);
    ~IoTSensor();

    void loop();
    virtual void doByInterval();
    void regEvent(String value, String consoleInfo);

    String getSubtype();
    String getID();

    unsigned long currentMillis;
    unsigned long prevMillis;
    unsigned long difference;

   protected:
    String _subtype;
    String _id;
    unsigned long _interval;
};
