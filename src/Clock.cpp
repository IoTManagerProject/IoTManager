#include "Clock.h"

#include "Global.h"

Clock* timeNow;
void clockInit() {
    timeNow = new Clock;
    timeNow->setNtpPool(jsonReadStr(configSetupJson, "ntp"));
    timeNow->setTimezone(jsonReadStr(configSetupJson, "timezone").toInt());
    ts.add(
        TIME_SYNC, 30000, [&](void*) {
            timeNow->hasSync();
        },
        nullptr, true);
    SerialPrint("I", F("NTP"), F("Clock Init"));
}
