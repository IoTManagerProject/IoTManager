
#include "utils/SerialPrint.h"
#include "classes/IoTRTC.h"

extern IoTRTC *watch;

void SerialPrint(String errorLevel, String module, String msg) {
    String tosend;
    if (watch) tosend = watch->gettime("d.m.y H:i:s");
    else tosend = prettyMillis(millis());

    tosend = tosend + " [" + errorLevel + "] [" + module + "] " + msg;
    Serial.println(tosend);

    if (isNetworkActive()) {
        if (jsonReadInt(settingsFlashJson, F("log")) != 0) {
            standWebSocket.broadcastTXT("/log|" + tosend);
        }
    }
}