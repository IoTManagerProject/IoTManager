#include "Broadcast.h"

#ifdef ESP8266
#include <ESPAsyncUDP.h>
#else
#include <AsyncUDP.h>
#endif

#include "Utils/PrintMessage.h"

static const char* MODULE = "Broadcast";

namespace Broadcast {

static const uint16_t UDP_PORT{4210};
static const IPAddress BROADCAST_IP{255, 255, 255, 255};

StringQueue* _income_messages = new StringQueue();
StringQueue* _outcome_messages = new StringQueue();
AsyncUDP udp;

void init() {
    if (!udp.listen(BROADCAST_IP, UDP_PORT)) {
        pm.error("unable to bind: " + String(UDP_PORT, DEC));
        return;
    }
    udp.onPacket([](AsyncUDPPacket packet) {
        String data = (char*)packet.data();
        IPAddress remoteIP = packet.remoteIP();
        uint16_t remotePort = packet.remotePort();
        pm.info("income " + prettyBytes(data.length()) + " from " + remoteIP.toString() + ":" + String(remotePort, DEC));
        if (data.length()) {
            _income_messages->push(data);
        }
    });
}

StringQueue* received() {
    return _income_messages;
}

void send(const String header, const String data) {
    String payload = header + ";" + data;
    pm.info(payload);
    _outcome_messages->push(payload);
}

void loop() {
    if (!_outcome_messages->available()) {
        return;
    }
    String payload;
    _outcome_messages->pop(payload);
    if (payload.isEmpty()) {
        return;
    }
    udp.broadcastTo(payload.c_str(), UDP_PORT);
}

}  // namespace Broadcast
