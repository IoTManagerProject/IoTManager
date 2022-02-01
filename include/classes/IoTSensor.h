#pragma once

#include <WString.h>

class IoTSensor {
   public:
    IoTSensor();
    ~IoTSensor();

    void loop();
    virtual void doByInterval();
    void init(String subtype, String id, unsigned long interval);
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

// extern IoTSensor* myIoTSensor;
//модулям не нужно знать эту переменную
//- ок понял