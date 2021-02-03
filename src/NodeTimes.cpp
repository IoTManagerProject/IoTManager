#include "Consts.h"
#ifdef GATE_MODE
#include "NodeTimes.h"
#include "items/vSensorNode.h"

void nodeTimesInit() {
    //   ts.add(
    //       TIMES, 60000, [&](void*) {
    //           if (configTimesJson != "{}") {
    //               String str = configTimesJson;
    //               str.replace("{", "");
    //               str.replace("}", "");
    //               str.replace("\"", "");
    //               str += ",";
    //               while (str.length() != 0) {
    //                   String tmp = selectToMarker(str, ",");
    //
    //                   String key = selectToMarker(tmp, ":");
    //                   int minutes = deleteBeforeDelimiter(tmp, ":").toInt();
    //                   minutes++;
    //                   if (key != "") {
    //                       jsonWriteStr(configTimesJson, key, String(minutes));
    //                       publishNodeInfo(minutes, key);
    //                   }
    //
    //                   str = deleteBeforeDelimiter(str, ",");
    //               }
    //           }
    //       },
    //       nullptr, true);
}

void publishTimes() {
    if (configTimesJson != "{}") {
        String str = configTimesJson;
        str.replace("{", "");
        str.replace("}", "");
        str.replace("\"", "");
        str += ",";
        while (str.length() != 0) {
            String tmp = selectToMarker(str, ",");
            String key = selectToMarker(tmp, ":");
            String minutes = deleteBeforeDelimiter(tmp, ":");
            if (key != "" && minutes != "") {
                if (mySensorNode != nullptr) {
                    for (unsigned int i = 0; i < mySensorNode->size(); i++) {
                        mySensorNode->at(i).setColors(key);
                    }
                }
            }
            str = deleteBeforeDelimiter(str, ",");
        }
    }
}

void publishNodeInfo(int minutes, String& key) {
//    if (minutes < NODE_ORANGE_COLOR_TIMEOUT) {
//        publishLastUpdateTime(key, String(minutes) + " min");
//        publishAnyJsonKey(key, "", "color");
//    } else if (minutes >= NODE_ORANGE_COLOR_TIMEOUT && minutes < NODE_RED_COLOR_TIMEOUT) {
//        publishLastUpdateTime(key, String(minutes) + " min");
//        publishAnyJsonKey(key, "orange", "color");
//    } else if (minutes >= NODE_RED_COLOR_TIMEOUT) {
//        publishLastUpdateTime(key, "offline");
//        publishAnyJsonKey(key, "red", "color");
//    }
}
#endif