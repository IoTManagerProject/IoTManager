#pragma once

#include <WString.h>

class IoTItem {
   public:
    IoTItem(String parameters);
    ~IoTItem();

    void loop();
    virtual void doByInterval();
    virtual void execute(String command, String param);

    void regEvent(String value, String consoleInfo);
    void regEvent(float value, String consoleInfo);

    String getSubtype();
    String getID();

    unsigned long currentMillis;
    unsigned long prevMillis;
    unsigned long difference;

   protected:
    String _subtype;
    String _id;
    unsigned long _interval;
    
    float _multiply;  // умножаем на значение
    float _plus;  // увеличиваем на значение
    int _map1;
    int _map2;
    int _map3;
    int _map4;
    int _round;  // 1, 10, 100, 1000, 10000
};
