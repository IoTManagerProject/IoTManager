
#include "Global.h"
#include "classes/IoTItem.h"
#include <Arduino.h>

#include "modules/sensors/SoftUart/SoftUart.h"

#ifdef ESP8266
SoftwareSerial* myUART = nullptr;
#else
HardwareSerial* myUART = nullptr;
#endif

class SoftUART : public IoTItem {
   private:
    int tx;
    int rx;
    int speed;

   public:
    SoftUART(String parameters) : IoTItem(parameters) {
        tx = jsonReadInt(parameters, "tx");
        rx = jsonReadInt(parameters, "rx");
        speed = jsonReadInt(parameters, "speed");

        if (!myUART) {
#ifdef ESP8266
            myUART = new SoftwareSerial(tx, rx);
            myUART->begin(speed);
#endif
#ifdef ESP32
            myUART = new HardwareSerial(2);
            myUART->begin(speed, SERIAL_8N1, rx, tx);
#endif
        }
        SerialPrint("i", F("UART"), F("UART Init"));
    }

    void uartHandle() {
        if (myUART) {
            static String incStr;
            if (myUART->available()) {
                char inc;
                inc = myUART->read();
                incStr += inc;
                if (inc == '\n') {
                    parse(incStr);
                    incStr = "";
                }
            }
        }
    }

    void parse(String& incStr) {
        SerialPrint("I", "=>UART", incStr);
    }
};

void* getAPI_SoftUART(String subtype, String param) {
    if (subtype == F("SoftUART")) {
        return new SoftUART(param);
    } else {
        return nullptr;
    }
}
