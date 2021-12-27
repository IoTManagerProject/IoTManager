#pragma once

#include <WString.h>

class IoTSensor {
    public:
        IoTSensor();
        virtual ~IoTSensor();

        void loop();
        virtual void doByInterval();
        void init(String key, String id, unsigned long interval);
        void regEvent(String value, String consoleInfo);

        String getKey();
        String getID();

        unsigned long currentMillis;
        unsigned long prevMillis;
        unsigned long difference;
    
    protected:
        String _key;
        String _id;
        unsigned long _interval;
};