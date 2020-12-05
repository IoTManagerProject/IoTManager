#include "SoftUART.h"

#ifdef ESP8266
SoftwareSerial* myUART = nullptr;
#else
HardwareSerial* myUART = nullptr;
#endif

void uartInit() {
    if (!myUART) {
#ifdef ESP8266
        myUART = new SoftwareSerial(4, 5);
        myUART->begin(9600);
#else
        myUART = new HardwareSerial(2);
        myUART->begin(4, 5);
#endif
    }
}

void uartHandle() {
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

void parse(String& incStr) {


}