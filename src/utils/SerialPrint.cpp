
#include "utils/SerialPrint.h"

void SerialPrint(String errorLevel, String module, String msg, String itemId) {
    String tosend = prettyMillis(millis());
    tosend = tosend + " [" + errorLevel + "] [" + module + "] " + msg;
    Serial.println(tosend);

    if (errorLevel == "E") {
        msg = cleanString(msg);
        // создаем событие об ошибке для возможной реакции в сценарии
        if (itemId != "") {
            IoTItems.push_back((IoTItem *)new externalVariable("{\"id\":\"" + itemId + "_onError\",\"val\":\"" + msg + "\",\"int\":1}"));
            generateEvent(itemId + "_onError", "1");
        } else {
            IoTItems.push_back((IoTItem *)new externalVariable("{\"id\":\"onError\",\"val\":\"" + module + " " + msg + "\",\"int\":1}"));
            generateEvent("onError", "1");
        }
    } 

    if (isNetworkActive()) {
        if (jsonReadInt(settingsFlashJson, F("log")) != 0) {
            // String pl = "/log|" + tosend;
            // standWebSocket.broadcastTXT(pl);
            sendStringToWs("corelg", tosend, -1);
        }
    }
}