#include "RemoteOrdersUdp.h"
#include <Arduino.h>
#include "Global.h"
#include "Class/NotAsync.h"
#include "Init.h"

#ifdef UDP_ENABLED
AsyncUDP asyncUdp;

void asyncUdpInit() {

    if (!jsonReadBool(configSetupJson, "onescen")) {
        return;
    }

    if (asyncUdp.listenMulticast(IPAddress(239, 255, 255, 255), 4210)) {
        asyncUdp.onPacket([](AsyncUDPPacket packet) {

            //Serial.print("UDP Packet Type: ");
            //Serial.println(packet.isBroadcast() ? "Broadcast" : packet.isMulticast() ? "Multicast" : "Unicast");
            //
            //Serial.print("From: ");
            //Serial.print(packet.remoteIP());
            //Serial.print(":");
            //Serial.println(packet.remotePort());
            //
            //Serial.print("To: ");
            //Serial.print(packet.localIP());
            //Serial.print(":");
            //Serial.println(packet.localPort());
            //Serial.print(", Length: ");
            //Serial.print(packet.length());
            //
            //Serial.print(", Data: ");
            //Serial.write(packet.data(), packet.length());

            String data = uint8tToString(packet.data(), packet.length());
            Serial.print("[i] [udp] Packet received: '");
            Serial.print(data);

            if (udpPacketValidation(data)) {
                Serial.println("', packet valid");
                udpPacketParse(data);
            }


            //else {
            //    //Serial.println("', Packet invalid");
            //}
            //reply to the client
            //String ip = WiFi.localIP().toString();
            //asyncUdp.broadcastTo(ip.c_str(), packet.remotePort());
            //packet.printf(ip.c_str(), packet.length());

            });
    }

    myNotAsyncActions->add(
        do_sendScenUDP, [&](void*) {

            String scen = "iotm;scen:";
            scen += readFile(String(DEVICE_SCENARIO_FILE), 2048);

            asyncUdp.broadcastTo(scen.c_str(), 4210);

        },
        nullptr);

    //ts.add(
    //UDP, 10000, [&](void*) {
    //Serial.println("sended");
    //asyncUdp.broadcastTo("iotm;Anyone here?", 4210); 
    //asyncUdp.broadcast("test");
    //asyncUdp.print("Hello Server!");
    //},
    //nullptr, true);

}

bool udpPacketValidation(String& data) {
    if (data.indexOf("iotm;") != -1) {
        return true;
    }
    else {
        return false;
    }
}

void udpPacketParse(String& data) {
    if (data.indexOf("scen:") != -1) {
        data = deleteBeforeDelimiter(data, ":");
        writeFile(String(DEVICE_SCENARIO_FILE), data);
        loadScenario();
    }
    else if (data.indexOf("event:") != -1) {
        data = deleteBeforeDelimiter(data, ":");
        eventBuf += data;
    }
}

String uint8tToString(uint8_t* data, size_t len) {
    String ret;
    while (len--) {
        ret += (char)*data++;
    }
    return ret;
}
#endif