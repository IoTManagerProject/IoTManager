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

AsyncUDP udp;
StringQueue* _income = new StringQueue();
StringQueue* _outcome = new StringQueue();

void init() {
    if (!udp.listen(BROADCAST_IP, UDP_PORT)) {
        pm.error("unable to bind: " + String(UDP_PORT, DEC));
        return;
    }

    udp.onPacket([](AsyncUDPPacket packet) {
        if (packet.length()) {
            size_t size = packet.length();
            char buf[size];
            uint8_t* ptr = packet.data();
            size_t i = 0;
            while (i < size) {
                buf[i++] = *(char*)ptr++;
            }
            buf[size] = '\x00';

            IPAddress remoteIP = packet.remoteIP();
            uint16_t remotePort = packet.remotePort();

            pm.info("<= " + remoteIP.toString() + ":" + String(remotePort, DEC) + " " + prettyBytes(size));

            _income->push(buf);
        }
    });
}

StringQueue* received() {
    return _income;
}

void send(const String header, const String data) {
    String payload = header + ";" + data;
    pm.info(payload);
    _outcome->push(payload);
}

void loop() {
    if (!_outcome->available()) {
        return;
    }
    String payload = _outcome->pop();

    if (payload.isEmpty()) {
        return;
    }
    udp.broadcastTo(payload.c_str(), UDP_PORT);
}

}  // namespace Broadcast
