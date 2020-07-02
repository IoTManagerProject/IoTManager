#include "Global.h"

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

String remote_ip;
String received;
int udp_period;
boolean _busy = false;

static unsigned int UDP_PORT = 4210;

//TODO Помомему тут ошибка в define'ах
void handleUdp_esp32();
void addKnownDevice(String fileName, String id, String dev_name, String ip);

void init() {
    removeFile("dev.csv");
    addFile("dev.csv", "device id;device name;ip address");

#ifdef ESP8266
    udp.begin(UDP_PORT);
#endif

    handleUdp_esp32();

    randomSeed(micros());
    udp_period = random(50000, 60000);

    ts.add(
        BROADCASTING, udp_period, [&](void*) {
            if (config.general()->isBroadcastEnabled() && !_busy) {
                String data = "iotm;";
                data += getChipId();
                data += ";";
                data += config.network()->getHostname();
                pm.info("send: " + data);
#ifdef ESP8266
                udp.beginPacketMulticast(MULTICAST_IP, UDP_PORT, WiFi.localIP());
                udp.write(data.c_str());
                udp.endPacket();
#else
                udp.broadcast(line_to_send.c_str());
#endif
            }
        },
        nullptr, false);
}

void loop() {
#ifdef ESP8266
    int packetSize = udp.parsePacket();
    if (!packetSize) {
        return;
    }

    char udp_packet[255];
    remote_ip = udp.remoteIP().toString();

    pm.info("incoming :" + prettyBytes(packetSize) + " from " + remote_ip + ":" + String(udp.remotePort(), DEC));

    int len = udp.read(udp_packet, 255);
    if (len) {
        udp_packet[len] = '\x00';
    }
    received = String(udp_packet);
    if (received.indexOf("iotm;") >= 0 || received.indexOf("mqttServer") >= 0) {
        udp_data_parse = true;
    }
#endif
    ;
}

void handleUdp_esp32() {
#ifdef ESP32
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

void do_udp_data_parse() {
    if (!udp_data_parse) {
        return;
    }

    if (received.indexOf("mqttServer") >= 0) {
        pm.info("mqtt settings");
        config.mqtt()->loadString(received);
        mqtt_restart_flag = true;
    }
    if (received.indexOf("iotm;") >= 0) {
        addKnownDevice("dev.csv", selectFromMarkerToMarker(received, ";", 1), selectFromMarkerToMarker(received, ";", 2), received);
    }
    udp_data_parse = false;
}

void addKnownDevice(String filename, String id, String dev_name, String ip) {
    auto file = seekFile("/" + filename);
    if (!file.find(id.c_str())) {
        addFile(filename, id + ";" + dev_name + ";  <a href=\"http://" + ip + "\" target=\"_blank\"\">" + ip + "</a>");
    }
}

void send_mqtt_settings() {
    _busy = true;

    String data = "{";
    data += "mqttServer\":\"" + config.mqtt()->getServer() + "\",";
    data += "mqttPort:" + String(config.mqtt()->getPort(), DEC) + ",";
    data += "mqttPrefix\":\"" + config.mqtt()->getPrefix() + "\",";
    data += "mqttUser\":\"" + config.mqtt()->getUser() + "\",";
    data += "mqttPass\":\"" + config.mqtt()->getPass() + "\"";
    data += "}";

    pm.info("mqtt: " + data);
#ifdef ESP8266
    udp.beginPacketMulticast(MULTICAST_IP, UDP_PORT, WiFi.localIP());
    udp.write(data.c_str());
    udp.endPacket();
#else
    udp.broadcast(data.c_str());
#endif
    _busy = false;
}

}  // namespace Broadcast