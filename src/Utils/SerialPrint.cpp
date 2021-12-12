#include "Utils\SerialPrint.h"

#include "Global.h"
#include "WebServer.h"

void SerialPrint(String errorLevel, String module, String msg) {
    String tosend = prettyMillis(millis()) + " [" + errorLevel + "] [" + module + "] " + msg;

    Serial.println(tosend);
    // mqttRootDevice +
    ws.textAll("/core/" + tosend);
}