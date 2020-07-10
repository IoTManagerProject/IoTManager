#include "Messages.h"

#include "Global.h"
#include "Config.h"
#include "Broadcast.h"
#include "Devices.h"
#include "Utils/SysUtils.h"

static const char* MODULE = "Messages";
StringQueue _income;
StringQueue _outcome;
namespace Messages {

StringQueue* outcome() {
    return &_outcome;
}

void post(const BroadcastMessage_t type, const String& content) {
    String buf = getMessageType(type);
    buf += ";";
    buf += content;
    if (!_outcome.push(buf)) {
        pm.error("outcome push");
    }
}

void income(const String& str) {
    if (!_income.push(str)) {
        pm.error("income push");
    }
}

void loop() {
    if (_income.available()) {
        parse(_income.pop());
    }
}

void parse(String buf) {
    BroadcastMessage_t type = getMessageType(getValue(buf, ';', 0).c_str());
    switch (type) {
        case BM_ANNOUNCE: {
            Devices::add(getValue(buf, ';', 1), getValue(buf, ';', 2), getValue(buf, ';', 3));
            break;
        }
        case BM_MQTT_SETTINGS:
            config.mqtt()->loadString(getValue(buf, ';', 1).c_str());
            perform_mqtt_restart();
            break;
        default:
            pm.error("unknown");
            break;
    }
    return;
}

}  // namespace Messages
