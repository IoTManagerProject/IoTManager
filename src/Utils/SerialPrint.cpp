#include "Utils\SerialPrint.h"

#include "Global.h"

void SerialPrint(String errorLevel, String module, String msg) {
    //if (module == "Stat") {
        Serial.println(prettyMillis(millis()) + " [" + errorLevel + "] [" + module + "] " + msg);
    //}
}