
#include "utils/SerialPrint.h"

void SerialPrint(const String& errorLevel, const String& module, const String& msg, const String& itemId) {
    String tosend = prettyMillis(millis());
    tosend += " [";
    tosend += errorLevel;
    tosend += "] [";
    tosend += module;
    tosend += "] ";
    tosend += msg;
    Serial.println(tosend);

    if (isNetworkActive()) {
        if (jsonReadInt(settingsFlashJson, F("log")) != 0) {
            sendStringToWs(F("corelg"), tosend, -1);
        }
    }

    if (errorLevel == "E") {
        cleanString(tosend);
        // создаем событие об ошибке для возможной реакции в сценарии
        if (itemId != "") {
            createItemFromNet(itemId + F("_onError"), tosend, -4);
        } else {
            // createItemFromNet("onError", tosend, -4);
        }
    }
}