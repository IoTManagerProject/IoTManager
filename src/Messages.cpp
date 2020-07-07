#include "Messages.h"

#include "Global.h"
#include "Consts.h"
#include "Config.h"
#include "Broadcast.h"
#include "Devices.h"
#include "NetworkManager.h"
#include "Utils/SysUtils.h"

static const char* MODULE = "Messages";

namespace Messages {

void loop() {
    if (!Broadcast::received()->available()) {
        return;
    }
    parseReceived(Broadcast::received());
}

void parseReceived(StringQueue* queue) {
    String buf = queue->pop();
    pm.info(buf);
    if (getValue(buf, ';', 0).equals(HEADER_ANNOUNCE)) {
        String deviceId = getValue(buf, ';', 1);
        String name = getValue(buf, ';', 2);
        String ip = getValue(buf, ';', 3);

        Devices::add(deviceId.c_str(), name.c_str(), ip.c_str());
        return;
    }

    if (getValue(buf, ';', 0).equals(HEADER_MQTT_SETTINGS)) {
        config.mqtt()->loadString(getValue(buf, ';', 1).c_str());
        perform_mqtt_restart();
        return;
    }

    pm.error("unknown");
}

// TODO Очередь отправки
void announce() {
    String data;
    data += getChipId();
    data += ";";
    data += config.general()->getBroadcastName();
    data += ";";
    data += NetworkManager::getHostIP().toString();
    data += ";";

    Broadcast::send(HEADER_ANNOUNCE, data);
}

}  // namespace Messages
