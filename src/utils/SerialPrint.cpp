
#include "utils/SerialPrint.h"

void SerialPrint(String errorLevel, String module, String msg) {
    String tosend;

    tosend = prettyMillis(millis());

    tosend = tosend + " [" + errorLevel + "] [" + module + "] " + msg;
    Serial.println(tosend);

    if (isNetworkActive()) {
        if (jsonReadInt(settingsFlashJson, F("log")) != 0) {
            // String pl = "/log|" + tosend;
            // standWebSocket.broadcastTXT(pl);
            sendStringToWs("corelg", tosend, -1);
        }
    }
}