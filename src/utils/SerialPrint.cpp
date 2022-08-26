
#include "utils/SerialPrint.h"

void SerialPrint(String errorLevel, String module, String msg) {
    String tosend;

    tosend = prettyMillis(millis());

    // if (module == "Loging") {
    tosend = tosend + " [" + errorLevel + "] [" + module + "] " + msg;
    Serial.println(tosend);

    if (isNetworkActive()) {
        if (jsonReadInt(settingsFlashJson, F("log")) != 0) {
            standWebSocket.broadcastTXT("/log|" + tosend);
        }
    }
    //}
}