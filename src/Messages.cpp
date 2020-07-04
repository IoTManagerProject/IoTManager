#include "Messages.h"

#include "Config.h"
#include "Broadcast.h"
#include "Devices.h"
#include "WiFiManager.h"

#include "Utils/SysUtils.h"

static const char* MODULE = "Messages";

namespace Messages {

void loop() {
    if (!Broadcast::received()->available()) {
        return;
    }
    process(Broadcast::received());
}

// TODO Очередь отправки
void announce() {
    String data;
    data += getChipId();
    data += ";";
    data += config.general()->getName();
    data += ";";
    data += WiFi.localIP().toString();
    data += ";";

    Broadcast::send(HEADER_ANNOUNCE, data);
}

void process(StringQueue* queue) {
    char buf[255];
    strncpy(buf, queue->pop().c_str(), sizeof(buf));
    size_t count = 0;
    char** tokens = str_split(buf, ';', count);
    if (!tokens || !count) {
        return;
    }
    if (strcmp(*tokens, HEADER_ANNOUNCE) == 0) {
        char* id = *(tokens + 1);
        char* name = *(tokens + 2);
        char* ip = *(tokens + 3);
        Devices::add(id, name, ip);
    } else if (strcmp(*tokens, HEADER_MQTT_SETTINGS) == 0) {
        char* json = *(tokens + 1);
        config.mqtt()->loadString(json);
        mqtt_restart_flag = true;
    } else {
        pm.error("unknown");
    }
    while (*tokens) {
        free(*(tokens++));
    }
    free(tokens);
}

}  // namespace Messages
