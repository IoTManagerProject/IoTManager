#include "Utils\SerialPrint.h"

#include "Global.h"
#include "HttpServer.h"

void SerialPrint(String errorLevel, String module, String msg) {
    Serial.println(prettyMillis(millis()) + " [" + errorLevel + "] [" + module + "] " + msg);

    String tosend = "[" + errorLevel + "] [" + module + "] " + msg;

    ws.textAll(tosend);
}