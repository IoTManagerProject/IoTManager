
#include "utils/SerialPrint.h"

void SerialPrint(String errorLevel, String module, String msg) {
    String tosend = prettyMillis(millis()) + " [" + errorLevel + "] [" + module + "] " + msg;
    Serial.println(tosend);

    if (jsonReadInt(settingsFlashJson, F("log")) != 0) {
        standWebSocket.broadcastTXT("/log|" + tosend);
    }
}