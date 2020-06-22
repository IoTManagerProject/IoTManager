#include "Clock.h"

#include "Utils/TimeUtils.h"

static const char* MODULE = "Clock";

void startTimeSync() {
    if (!hasTimeSynced()) {
        pm.info("Start syncing");
        reconfigTime();
    }
}

void setupSntp() {
    int tzs = getBiasInSeconds();
    int tzh = tzs / 3600;
    tzs -= tzh * 3600;
    int tzm = tzs / 60;
    tzs -= tzm * 60;

    String ntp = jsonReadStr(configSetupJson, "ntp");
    pm.info("Setup ntp: " + ntp);
    char tzstr[64];
    snprintf(tzstr, sizeof tzstr, "ESPUSER<%+d:%02d:%02d>", tzh, tzm, tzs);
    return configTime(tzstr, ntp.c_str(), "pool.ntp.org", "time.nist.gov");
}

void reconfigTime() {
#ifdef ESP32
    uint8_t i = 0;
    struct tm timeinfo;
    while (!getLocalTime(&timeinfo) && i <= 4) {
        Serial.print(".");
        i++;
        delay(1000);
    }
#endif

#ifdef ESP8266
    setupSntp();
    uint8_t i = 0;
    while (!hasTimeSynced() && i < 4) {
        Serial.print(".");
        i++;
        delay(300);
    }
#endif

    if (getTimeUnix() != "failed") {
        pm.info("Time synced " + getDateDigitalFormated() + " " + getTime());
    } else {
        pm.error("Failed to obtain");
    }
}
