#include "Utils\SerialPrint.h"

/*
Before:
RAM:   [=         ]  12.4% (used 40616 bytes from 327680 bytes)
Flash: [=======   ]  65.7% (used 860718 bytes from 1310720 bytes)

After:

RAM:   [=         ]  12.4% (used 40664 bytes from 327680 bytes)
Flash: [=======   ]  65.4% (used 856798 bytes from 1310720 bytes)

After:
RAM:   [=         ]  12.4% (used 40664 bytes from 327680 bytes)
Flash: [=======   ]  65.3% (used 855546 bytes from 1310720 bytes)
*/

const char* L_MODULE[] = {"WiFi", "MQTT", "WebServer", "Scenario"};

// TODO управление печатью по уровняню, модулю    
void SerialPrint(char errorLevel, const String& module, const String& msg) {   
    char buf[32];
    Serial.printf("%8s [%C] [%12s] %s\r\n", prettySeconds(millis() / 1000, buf, sizeof(buf)), errorLevel, module.c_str(), msg.c_str());
}

void SerialPrint(char errorLevel, module_t module, const String& msg) {
    SerialPrint(errorLevel, L_MODULE[module], msg);
}
