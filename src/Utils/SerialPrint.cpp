#include "Utils\SerialPrint.h"

#include "Global.h"
#include "WebServer.h"
#include "WebSocket.h"

void SerialPrint(String errorLevel, String module, String msg) {
    String tosend = prettyMillis(millis()) + " [" + errorLevel + "] [" + module + "] " + msg;
    Serial.println(tosend);
    // wsPublishData(F("log"), tosend);
}