#ifdef uartEnable
#include "SoftUART.h"
#include "Global.h"

#ifdef ESP8266
SoftwareSerial* myUART = nullptr;
#else
HardwareSerial* myUART = nullptr;
#endif

void uartInit() {
    if (!jsonReadBool(configSetupJson, "uartEnable")) {
        return;
    }
    if (!myUART) {
#ifdef ESP8266
        myUART = new SoftwareSerial(jsonReadInt(configSetupJson, "uartTX"), jsonReadInt(configSetupJson, "uartRX"));
        myUART->begin(jsonReadInt(configSetupJson, "uartS"));
#else
        myUART = new HardwareSerial(2);
        myUART->begin(4, 5);
#endif
    }
    SerialPrint("I", F("UART"), F("UART Init"));
}

void uartHandle() {
    if (myUART) {
        if (!jsonReadBool(configSetupJson, "uartEnable")) {
            return;
        }
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
    incStr.replace("\r\n", "");
    incStr.replace("\r", "");
    incStr.replace("\n", "");
    if (incStr.indexOf("set") != -1) {
        incStr = deleteBeforeDelimiter(incStr, " ");
        orderBuf += incStr;
        SerialPrint("I", "=>UART", incStr);
    }
}
#endif