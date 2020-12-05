#include "SoftUART.h"

SoftwareSerial* myUART{ nullptr };

void uartInit() {
    if (!myUART) {
        myUART = new SoftwareSerial(4, 5);
    }
}

void uartHandle() {
    static String incStr;
    if (myUART->available()) {
        char inc;
        inc = myUART->read();
        incStr += inc;
        if (inc == 0x0A) {
            parse(incStr);
            incStr = "";
        }
    }
}

void parse(String& incStr) {

    
}