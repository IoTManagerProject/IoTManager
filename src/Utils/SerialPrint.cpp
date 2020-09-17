#include "Utils\SerialPrint.h"

#include "Global.h"


void SerialPrint(String errorLevel, String module, String msg) {
    Serial.println(prettyMillis(millis()) + " [" + errorLevel + "] [" + module + "] " + msg);
}