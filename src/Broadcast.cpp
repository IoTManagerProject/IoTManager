#include "Global.h"

#include "Devices.h"

static const char* MODULE = "Broadcast";

namespace Broadcast {

#ifdef ESP8266
IPAddress MULTICAST_IP(255, 255, 255, 255);
WiFiUDP udp;
#endif

#ifdef ESP32
IPAddress MULTICAST_IP(239, 255, 255, 255);
AsyncUDP udp;
#endif

StringQueue _received;
boolean _busy = false;

static unsigned int UDP_PORT = 4210;

void init() {
    udp.begin(UDP_PORT);
}

void loop() {
    String remote_ip;
#ifdef ESP8266
    char udp_packet[255];
    int packetSize = udp.parsePacket();
    if (!packetSize) {
        return;
    }
    remote_ip = udp.remoteIP().toString();
    pm.info("incoming: " + remote_ip + ":" + String(udp.remotePort(), DEC) + " " + prettyBytes(packetSize));
    int len = udp.read(udp_packet, 255);
    if (len) {
        udp_packet[len] = '\x00';
    }
    _received.push(String(udp_packet));
#else
    if (udp.listenMulticast(udp_multicastIP, udp_port)) {
        udp.onPacket([](AsyncUDPPacket packet) {
            received = (char*)packet.data();
            remote_ip = packet.remoteIP().toString();
            if (jsonReadStr(configSetupJson, "udponoff") == "1") {
                if (received.indexOf("iotm;") >= 0) {
                    udp_data_parse = true;
                }
                if (received.indexOf("mqttServer") >= 0) {
                    udp_data_parse = true;
                }
            }
        });
    }
#endif
}

StringQueue* received() {
    return &_received;
}

void send(const String header, const String data) {
    _busy = true;
#ifdef ESP8266
    String payload = header + ";" + data;
    pm.info(payload);
    udp.beginPacketMulticast(MULTICAST_IP, UDP_PORT, WiFi.localIP());
    udp.write(payload.c_str());
    udp.endPacket();
#else
    udp.broadcast(data.c_str());
#endif
    _busy = false;
}

}  // namespace Broadcast