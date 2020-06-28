#include "Global.h"

#ifdef ESP8266
IPAddress udp_multicastIP(255, 255, 255, 255);
WiFiUDP Udp;
#endif
#ifdef ESP32
IPAddress udp_multicastIP(239, 255, 255, 255);
AsyncUDP udp;
#endif
String received_ip;
String received_udp_line;
int udp_period;
boolean udp_busy = false;
unsigned int udp_port = 4210;

void handleUdp_esp32();
void add_dev_in_list(String fileName, String id, String dev_name, String ip);

#ifdef UDP_ENABLED
void UDP_init() {
    removeFile("dev.csv");
    addFile("dev.csv", "device id;device name;ip address");

#ifdef ESP8266
    Udp.begin(udp_port);
#endif

    handleUdp_esp32();

    randomSeed(micros());
    udp_period = random(50000, 60000);

    ts.add(
        UDP, udp_period, [&](void*) {
            if (jsonReadStr(configSetupJson, "udponoff") == "1") {
                if (WiFi.status() == WL_CONNECTED) {
                    if (!udp_busy) {
                        String line_to_send = "iotm;" + chipId + ";" + jsonReadStr(configSetupJson, "name");
#ifdef ESP8266
                        Udp.beginPacketMulticast(udp_multicastIP, udp_port, WiFi.localIP());
                        Udp.write(line_to_send.c_str());
                        Udp.endPacket();
#endif
#ifdef ESP32
                        udp.broadcast(line_to_send.c_str());
#endif
                        Serial.println("[UDP<=] dev info send");
                    }
                }
            }
        },
        nullptr, false);
}

void loopUdp() {
#ifdef ESP8266
    if (jsonReadStr(configSetupJson, "udponoff") == "1") {
        if (WiFi.status() == WL_CONNECTED) {
            int packetSize = Udp.parsePacket();
            if (packetSize) {
                char udp_incomingPacket[255];
                Serial.printf("[UDP=>] Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
                received_ip = Udp.remoteIP().toString();
                int len = Udp.read(udp_incomingPacket, 255);
                if (len > 0) {
                    udp_incomingPacket[len] = 0;
                }
                received_udp_line = String(udp_incomingPacket);

                if (received_udp_line.indexOf("iotm;") >= 0) {
                    udp_data_parse = true;
                }
                if (received_udp_line.indexOf("mqttServer") >= 0) {
                    udp_data_parse = true;
                }
            }
        }
    }
#endif
}

void handleUdp_esp32() {
#ifdef ESP32
    if (udp.listenMulticast(udp_multicastIP, udp_port)) {
        udp.onPacket([](AsyncUDPPacket packet) {
            received_udp_line = (char*)packet.data();
            received_ip = packet.remoteIP().toString();
            if (jsonReadStr(configSetupJson, "udponoff") == "1") {
                if (received_udp_line.indexOf("iotm;") >= 0) {
                    udp_data_parse = true;
                }
                if (received_udp_line.indexOf("mqttServer") >= 0) {
                    udp_data_parse = true;
                }
            }
        });
    }
#endif
}

void do_udp_data_parse() {
    if (udp_data_parse) {
        udp_data_parse = false;
        Serial.print("[UDP=>] " + received_ip);
        Serial.print(" ");
        Serial.println(received_udp_line);
        if (received_udp_line.indexOf("mqttServer") >= 0) {
            jsonWriteStr(configSetupJson, "mqttServer", jsonReadStr(received_udp_line, "mqttServer"));
            jsonWriteInt(configSetupJson, "mqttPort", jsonReadInt(received_udp_line, "mqttPort"));
            jsonWriteStr(configSetupJson, "mqttPrefix", jsonReadStr(received_udp_line, "mqttPrefix"));
            jsonWriteStr(configSetupJson, "mqttUser", jsonReadStr(received_udp_line, "mqttUser"));
            jsonWriteStr(configSetupJson, "mqttPass", jsonReadStr(received_udp_line, "mqttPass"));
            saveConfig();
            Serial.println("[V] new mqtt setting received from udp and saved");
            mqttParamsChanged = true;
        }
        if (received_udp_line.indexOf("iotm;") >= 0) {
            add_dev_in_list("dev.csv", selectFromMarkerToMarker(received_udp_line, ";", 1), selectFromMarkerToMarker(received_udp_line, ";", 2), received_ip);
        }
    }
}

void add_dev_in_list(String filename, String id, String dev_name, String ip) {
    auto file = seekFile("/" + filename);
    if (!file.find(id.c_str())) {
        addFile(filename, id + ";" + dev_name + "; <a href=\"http://" + ip + "\" target=\"_blank\"\">" + ip + "</a>");
    }
}

void send_mqtt_to_udp() {
    if (jsonReadStr(configSetupJson, "udponoff") == "1") {
        if (WiFi.status() == WL_CONNECTED) {
            udp_busy = true;
            String mqtt_data = "{}";
            jsonWriteStr(mqtt_data, "mqttServer", jsonReadStr(configSetupJson, "mqttServer"));
            jsonWriteInt(mqtt_data, "mqttPort", jsonReadInt(configSetupJson, "mqttPort"));
            jsonWriteStr(mqtt_data, "mqttPrefix", jsonReadStr(configSetupJson, "mqttPrefix"));
            jsonWriteStr(mqtt_data, "mqttUser", jsonReadStr(configSetupJson, "mqttUser"));
            jsonWriteStr(mqtt_data, "mqttPass", jsonReadStr(configSetupJson, "mqttPass"));
            Serial.println(mqtt_data);
#ifdef ESP8266
            Udp.beginPacketMulticast(udp_multicastIP, udp_port, WiFi.localIP());
            Udp.write(mqtt_data.c_str());
            Udp.endPacket();
#endif
#ifdef ESP32
            udp.broadcast(mqtt_data.c_str());
#endif
            Serial.println("[UDP<=] mqtt info send");
            udp_busy = false;
        }
    }
}

void do_mqtt_send_settings_to_udp() {
    if (mqtt_send_settings_to_udp) {
        mqtt_send_settings_to_udp = false;
        send_mqtt_to_udp();
    }
}
#endif