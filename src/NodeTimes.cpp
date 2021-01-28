#include "NodeTimes.h"

void nodeTimesInit() {
    ts.add(
        TIMES, 60000, [&](void*) {
            if (configTimesJson != "{}") {
                String str = configTimesJson;
                str.replace("{", "");
                str.replace("}", "");
                str.replace("\"", "");
                str += ",";
                while (str.length() != 0) {
                    String tmp = selectToMarker(str, ",");
                    String key = selectToMarker(tmp, ":");
                    int minutes = deleteBeforeDelimiter(tmp, ":").toInt();
                    minutes++;
                    if (key != "") {
                        jsonWriteStr(configTimesJson, key, String(minutes));
                        publishNodeInfo(minutes, key);
                    }
                    str = deleteBeforeDelimiter(str, ",");
                }
            }
        },
        nullptr, true);
}

void publishTimes() {
    if (configTimesJson != "{}") {
        String str = configTimesJson;
        str.replace("{", "");
        str.replace("}", "");
        str.replace("\"", "");
        str += ",";
        Serial.println(str);
        while (str.length() != 0) {
            String tmp = selectToMarker(str, ",");
            String key = selectToMarker(tmp, ":");
            String minutes = deleteBeforeDelimiter(tmp, ":");
            Serial.println(key + " " + minutes);
            if (key != "" && minutes != "") {
                publishNodeInfo(minutes.toInt(), key);
            }
            str = deleteBeforeDelimiter(str, ",");
        }
    }
}

void publishNodeInfo(int minutes, String& key) {
    if (minutes < NODE_ORANGE_COLOR_TIMEOUT) {
        publishLastUpdateTime(key, String(minutes) + " min");
        publishAnyJsonKey(key, "", "color");
    } else if (minutes >= NODE_ORANGE_COLOR_TIMEOUT && minutes < NODE_RED_COLOR_TIMEOUT) {
        publishLastUpdateTime(key, String(minutes) + " min");
        publishAnyJsonKey(key, "orange", "color");
    } else if (minutes >= NODE_RED_COLOR_TIMEOUT) {
        publishLastUpdateTime(key, "offline");
        publishAnyJsonKey(key, "red", "color");
    }
}