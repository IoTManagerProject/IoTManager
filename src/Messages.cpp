#include "Messages.h"

#include "StringCommand.h"
#include "Global.h"
#include "Config.h"
#include "Broadcast.h"
#include "Devices.h"
#include "Utils/SysUtils.h"

static const char* MODULE = "Messages";

namespace Messages {
bool _ready;
StringQueue _income;
StringQueue _outcome;
StringCommand _cmd{";"};

bool available() {
    return _outcome.available();
}

void post(const BroadcastMessage_t type, const String& content) {
    String buf = getMessageType(type);
    buf += ";";
    buf += content;
    _outcome.push(buf);
}

void outcome(String& str) {
    _outcome.pop(str);
}

void income(const String str) {
    _income.push(str);
}

void cmd_device() {
    Devices::add(_cmd.next(), _cmd.next(), _cmd.next());
}

void cmd_mqttSettings() {
    config.mqtt()->loadString(_cmd.next());
    perform_mqtt_restart();
}

void onUnknownCommand(const char* str) {
    pm.error(str);
}

void init() {
    _cmd.addCommand(getMessageType(BM_ANNOUNCE), cmd_device);
    _cmd.addCommand(getMessageType(BM_MQTT_SETTINGS), cmd_mqttSettings);
    _cmd.setDefaultHandler(onUnknownCommand);
}

void loop() {
    if (!_ready) {
        init();
        _ready = true;
    }
    if (_income.available()) {
        String buf;
        _income.pop(buf);
        pm.info(buf);
        _cmd.readStr(buf);
    }
}

}  // namespace Messages
