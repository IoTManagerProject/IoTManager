#include "Global.h"
#include "classes/IoTItem.h"

#ifdef ESP8266
#include <SoftwareSerial.h>
#else
#include <HardwareSerial.h>
#endif

class DwinI : public IoTItem {
   private:
#ifdef ESP8266
    SoftwareSerial* _myUART;
#else
    HardwareSerial* _myUART;
#endif

   public:
    DwinI(String parameters) : IoTItem(parameters) {
        int _tx, _rx, _speed, _line;
        jsonRead(parameters, "RX", _rx);
        jsonRead(parameters, "TX", _tx);
        jsonRead(parameters,  "speed", _speed);
        jsonRead(parameters,  "line", _line);

#ifdef ESP8266
        _myUART = new SoftwareSerial(_rx, _tx);
        _myUART->begin(_speed);
#endif
#ifdef ESP32
        _myUART = new HardwareSerial(_line);
        _myUART->begin(_speed, SERIAL_8N1, _rx, _tx);
#endif
    }

    void doByInterval() {
        Serial.println("ddddddddddddd");
    }

    IoTValue execute(String command, std::vector<IoTValue> &param) {  // будет возможным использовать, когда сценарии запустятся
        
        return {};
    }

    ~DwinI(){
        
    };
};

void *getAPI_DwinI(String subtype, String param) {
    if (subtype == F("DwinI")) {
        return new DwinI(param);
    } else {
        return nullptr;
    }
}
