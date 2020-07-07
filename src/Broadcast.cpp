#include "Broadcast.h"

#ifdef ESP8266
#include <ESPAsyncUDP.h>
#else
#include <AsyncUDP.h>
#endif

#include "NetworkManager.h"
#include "Utils/PrintMessage.h"

static const char* MODULE = "Broadcast";

namespace Broadcast {

static const uint16_t UDP_PORT{4210};
// static const IPAddress BROADCAST_IP{255, 255, 255, 255};

AsyncUDP udp;
StringQueue* _income = new StringQueue();
StringQueue* _outcome = new StringQueue();
bool _initialized = false;

bool init() {
    if (!NetworkManager::isNetworkActive()) {
        return false;
    }
    if (!udp.listen(UDP_PORT)) {
        pm.error("unable to bind: " + String(UDP_PORT, DEC));
        return false;
    } else {
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

                String info_msg = "<= ";
                info_msg += packet.isBroadcast() ? "broad" : packet.isMulticast() ? "multi" : "uni";
                info_msg += " ";
                info_msg += remoteIP.toString() + ":" + String(remotePort, DEC);
                info_msg += " ";
                info_msg += prettyBytes(size);
                pm.info(std::move(info_msg));

                if (!_income->push(buf)) {
                    pm.error("push to incoming");
                }
            }
        });
        return true;
    }
}

StringQueue* received() {
    return _income;
}

void send(const String header, const String data) {
    String payload = header + ";" + data;
    pm.info("=> " + payload);

    if (!_outcome->push(payload)) {
        pm.error("push to outcome");
    }
}

void loop() {
    if (!_initialized) {
        _initialized = init();
        return;
    }
    if (_outcome->available()) {
        String payload = _outcome->pop();
        if (payload.isEmpty()) {
            return;
        }
        udp.broadcast(payload.c_str());
    }
}
}  // namespace Broadcast
