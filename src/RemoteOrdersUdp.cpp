#include "RemoteOrdersUdp.h"
#include <Arduino.h>
#include "Global.h"

#ifdef UDP_ENABLED
AsyncUDP asyncUdp;

void asyncUdpInit() {
    if (asyncUdp.listenMulticast(IPAddress(239, 255, 255, 255), 4210)) {
        asyncUdp.onPacket([](AsyncUDPPacket packet) {
            Serial.print("UDP Packet Type: ");
            Serial.println(packet.isBroadcast() ? "Broadcast" : packet.isMulticast() ? "Multicast" : "Unicast");

            Serial.print("From: ");
            Serial.print(packet.remoteIP());
            Serial.print(":");
            Serial.println(packet.remotePort());

            Serial.print("To: ");
            Serial.print(packet.localIP());
            Serial.print(":");
            Serial.println(packet.localPort());

            //Serial.print(", Length: ");
            //Serial.print(packet.length());
            //
            //Serial.print(", Data: ");
            //Serial.write(packet.data(), packet.length());

            String data = uint8tToString(packet.data(), packet.length());
            Serial.print("[i] [udp] Packet received: '");
            Serial.print(data);
            if (udpPacketValidation(data)) {
                udpPacketParse(data);
                //Serial.println("', Packet valid");
            }
            else {
                //Serial.println("', Packet invalid");
            }

            //reply to the client

            String ip = WiFi.localIP().toString();
            asyncUdp.broadcastTo(ip.c_str(), packet.remotePort());

            //packet.printf(ip.c_str(), packet.length());

            });
    }

    ts.add(
        UDP, 10000, [&](void*) {

            Serial.println("sended");
            asyncUdp.broadcastTo("Anyone here?", 64130); 
            //asyncUdp.broadcast("test");
            //asyncUdp.print("Hello Server!");

        },
        nullptr, true);

}

String uint8tToString(uint8_t* data, size_t len) {
    String ret;
    while (len--) {
        ret += (char)*data++;
    }
    return ret;
}

bool udpPacketValidation(String& data) {
    if (data.indexOf("iotm;") != -1 && data.indexOf(getChipId()) != -1) {
        return true;
    }
    else {
        return false;
    }
}

//iotm;chipid;button-out-1_1
void udpPacketParse(String& data) {
    data = selectFromMarkerToMarker(data, ";", 2);
    data.replace("_", " ");
    orderBuf += data + ",";
}
#endif