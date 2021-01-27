#include "deviceTimes.h"

void myTimesInit() {
    ts.add(
        TIMES, 60000, [&](void *) {
            if (configTimesJson != "{}") {
                String str = configTimesJson;

                str.replace("{", "");
                str.replace("}", "");
                str.replace("\"", "");
                str += ",";

                while (str.length() != 0) {
                    String tmp = selectToMarker(str, ",");

                    String key = selectToMarker(tmp, ":");
                    int state = deleteBeforeDelimiter(tmp, ":").toInt();

                    state++;
                    if (key != "") {
                        jsonWriteStr(configTimesJson, key, String(state));
                        publishLastUpdateTime(key, String(state));
                        if (state >= 60) {
                            publishAnyJsonKey(key, "red", "color");
                        } else {
                            publishAnyJsonKey(key, "", "color");
                        }
                    }
                    str = deleteBeforeDelimiter(str, ",");
                }
            }
        },
        nullptr, true);
}