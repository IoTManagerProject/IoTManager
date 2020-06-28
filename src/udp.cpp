#include "Global.h"

static const char* MODULE = "Udp";

#ifdef ESP8266
IPAddress udp_multicastIP(255, 255, 255, 255);
WiFiUDP udp;
#endif
#ifdef ESP32
IPAddress udp_multicastIP(239, 255, 255, 255);
AsyncUDP udp;
#endif
String remote_ip;
String received;
int udp_period;
boolean udp_busy = false;
unsigned int udp_port = 4210;

//TODO Помомему тут ошибка в define'ах
void handleUdp_esp32();

bool isUdpEnabled() {
    return jsonReadBool(configSetupJson, "udponoff") && isNetworkActive();
}

void add_dev_in_list(String fileName, String id, String dev_name, String ip);

#ifdef UDP_ENABLED
void udp_init() {
    removeFile(String("dev.csv"));
    addFile(String("dev.csv"), String("device id;device name;ip address"));

#ifdef ESP8266
    udp.begin(udp_port);
#endif

    handleUdp_esp32();

    randomSeed(micros());
    udp_period = random(50000, 60000);

    ts.add(
        UDP, udp_period, [&](void*) {
            if (isUdpEnabled() && !udp_busy) {
                pm.info(String("sending info"));
                String payload = "iotm;";
                payload += chipId;
                payload += ";";
                payload += jsonReadStr(configSetupJson, "name");
#ifdef ESP8266
                udp.beginPacketMulticast(udp_multicastIP, udp_port, WiFi.localIP());
                udp.write(payload.c_str());
                udp.endPacket();
#endif
#ifdef ESP32
                udp.broadcast(line_to_send.c_str());
#endif
            }
        },
        nullptr, false);
}

void loopUdp() {
#ifdef ESP8266
    if (!isUdpEnabled()) {
        return;
    }

    int packetSize = udp.parsePacket();
    if (!packetSize) {
        return;
    }

    char udp_packet[255];
    remote_ip = udp.remoteIP().toString();

    pm.info(prettyBytes(packetSize) + " from " + remote_ip + ":" + String(udp.remotePort(), DEC));

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
        pm.info(String("received settings"));
        jsonWriteStr(configSetupJson, "mqttServer", jsonReadStr(received, "mqttServer"));
        jsonWriteInt(configSetupJson, "mqttPort", jsonReadInt(received, "mqttPort"));
        jsonWriteStr(configSetupJson, "mqttPrefix", jsonReadStr(received, "mqttPrefix"));
        jsonWriteStr(configSetupJson, "mqttUser", jsonReadStr(received, "mqttUser"));
        jsonWriteStr(configSetupJson, "mqttPass", jsonReadStr(received, "mqttPass"));
        saveConfig();
        mqttParamsChanged = true;
    }
    if (received.indexOf("iotm;") >= 0) {
        add_dev_in_list("dev.csv", selectFromMarkerToMarker(received, ";", 1), selectFromMarkerToMarker(received, ";", 2), received);
    }
    udp_data_parse = false;
}

void add_dev_in_list(String filename, String id, String dev_name, String ip) {
    auto file = seekFile("/" + filename);
    if (!file.find(id.c_str())) {
        addFile(filename, id + ";" + dev_name + "; <a href=\"http://" + ip + "\" target=\"_blank\"\">" + ip + "</a>");
    }
}

void send_mqtt_to_udp() {
    if (!isUdpEnabled()) {
        return;
    }
    udp_busy = true;
    String data = "{}";
    jsonWriteStr(data, "mqttServer", jsonReadStr(configSetupJson, "mqttServer"));
    jsonWriteInt(data, "mqttPort", jsonReadInt(configSetupJson, "mqttPort"));
    jsonWriteStr(data, "mqttPrefix", jsonReadStr(configSetupJson, "mqttPrefix"));
    jsonWriteStr(data, "mqttUser", jsonReadStr(configSetupJson, "mqttUser"));
    jsonWriteStr(data, "mqttPass", jsonReadStr(configSetupJson, "mqttPass"));
#ifdef ESP8266
    udp.beginPacketMulticast(udp_multicastIP, udp_port, WiFi.localIP());
    udp.write(data.c_str());
    udp.endPacket();
#endif
#ifdef ESP32
    udp.broadcast(mqtt_data.c_str());
#endif
    pm.info(String("sent info"));
    udp_busy = false;
}

void do_mqtt_send_settings_to_udp() {
    if (mqtt_send_settings_to_udp) {
        mqtt_send_settings_to_udp = false;
        send_mqtt_to_udp();
    }
}
#endif