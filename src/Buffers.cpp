//#include "Buffers.h"

//генеирует событие
// void eventGen2(String eventName, String eventValue) {
//     if (!jsonReadBool(settingsFlashJson, "scen")) {
//         return;
//     }
//     String event = eventName + " " + eventValue + ",";
//     eventBuf += event;

//     SerialPrint("i", "Event add", eventName + " " + eventValue);

//     if (jsonReadBool(settingsFlashJson, "MqttOut")) {
//         if (eventName != "timenow") {
//             publishEvent(eventName, eventValue);
//         }
//     }
// }

// void spaceCmdExecute(String& cmdStr) {
//     cmdStr += "\r\n";
//     cmdStr.replace("\r\n", "\n");
//     cmdStr.replace("\r", "\n");
//     while (cmdStr.length()) {
//         String buf = selectToMarker(cmdStr, "\n");
//         if (buf != "") {
//             sCmd.readStr(buf);
//             SerialPrint("i", F("Order done W"), buf);
//         }
//         cmdStr = deleteBeforeDelimiter(cmdStr, "\n");
//     }
// }

// String getValueJson(String& key) {
//     String live = jsonReadStr(paramsHeapJson, key);
//     String store = jsonReadStr(paramsFlashJson, key);
//     if (live != nullptr) {
//         return live;
//     } else if (store != nullptr) {
//         return store;
//     } else if (store == nullptr && live == nullptr) {
//         return "no value";
//     } else {
//         return "data error";
//     }
// }
