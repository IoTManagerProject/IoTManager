#include "Consts.h"
#ifdef EnableUart
#include "BufferExecute.h"
#include "Global.h"
#include "SoftUART.h"

#ifdef ESP8266
SoftwareSerial* myUART = nullptr;
#else
HardwareSerial* myUART = nullptr;
#endif

void uartInit() {
    if (!jsonReadBool(configSetupJson, "uart")) {
        return;
    }
    if (!myUART) {
#ifdef ESP8266
        myUART = new SoftwareSerial(jsonReadInt(configSetupJson, "uartTX"), jsonReadInt(configSetupJson, "uartRX"));
        myUART->begin(jsonReadInt(configSetupJson, "uartS"));
#endif        
#ifdef ESP32
        myUART = new HardwareSerial(2);
        long speed = jsonReadInt(configSetupJson, "uartS");       
        int rxPin = jsonReadInt(configSetupJson, "uartRX");
        int txPin = jsonReadInt(configSetupJson, "uartTX");
        myUART->begin(speed, SERIAL_8N1, rxPin, txPin);
#endif
    }
    SerialPrint("I", F("UART"), F("UART Init"));
}

void uartHandle() {
    if (myUART) {
        if (!jsonReadBool(configSetupJson, "uart")) {
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
        loopCmdAdd(incStr);
        SerialPrint("I", "=>UART", incStr);
    }
}
#endif