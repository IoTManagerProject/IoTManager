#include "Broadcast.h"

#ifdef ESP8266
#include <ESPAsyncUDP.h>
#else
#include <AsyncUDP.h>
#endif

#include "NetworkManager.h"
#include "Messages.h"
#include "Utils/PrintMessage.h"

static const char* MODULE = "Broadcast";

namespace Broadcast {

static const uint16_t UDP_PORT{4210};
// static const IPAddress BROADCAST_IP{255, 255, 255, 255};

AsyncUDP udp;
bool _ready = false;
bool _busy = false;

bool init() {
    if (!NetworkManager::isNetworkActive()) {
        return false;
    }
    if (!udp.listen(UDP_PORT)) {
        pm.error("on bind: " + String(UDP_PORT, DEC));
        return false;
    } else {
        udp.onPacket([](AsyncUDPPacket packet) {
            _busy = true;
            if (packet.length()) {
                size_t size = packet.length();
                char buf[size + 1];
                uint8_t* ptr = packet.data();
                size_t i = 0;
                while (i < size) {
                    buf[i++] = *(char*)ptr++;
                }
                buf[size] = '\x00';

                IPAddress remoteIP = packet.remoteIP();
                uint16_t remotePort = packet.remotePort();

                String info_msg = packet.isBroadcast() ? "b" : packet.isMulticast() ? "m" : "u";
                info_msg += " <= ";
                info_msg += remoteIP.toString() + ":" + String(remotePort, DEC);
                info_msg += " ";
                info_msg += prettyBytes(size);

                pm.info(info_msg);

                Messages::income(buf);
            }
            _busy = false;
        });

        return true;
    }
}

void loop() {
    if (!_ready) {
        _ready = init();
        return;
    }
    if (!_busy) {
        if (Messages::available()) {
            String buf;
            Messages::outcome(buf);
            if (!buf.isEmpty()) {
                udp.broadcast(buf.c_str());
            }
        }
    }
}
}  // namespace Broadcast
