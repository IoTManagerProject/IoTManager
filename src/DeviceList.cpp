#include "DeviceList.h"

void addThisDeviceToList() {
    jsonWriteStr_(devListHeapJson, F("devicelist"), "");  //метка для парсинга
    jsonWriteStr_(devListHeapJson, F("ip"), jsonReadStr(settingsFlashJson, F("ip")));
    jsonWriteStr_(devListHeapJson, F("id"), jsonReadStr(settingsFlashJson, F("id")));
    jsonWriteStr_(devListHeapJson, F("name"), jsonReadStr(settingsFlashJson, F("name")));

    // для проверки
    // devListHeapJson = devListHeapJson + ",";
    // String test;
    // jsonWriteStr_(test, "ip", "192.168.88.88");
    // jsonWriteStr_(test, "id", "123456789");
    // jsonWriteStr_(test, "name", "test");
    // devListHeapJson = devListHeapJson + test;

    devListHeapJson = "[" + devListHeapJson + "]";
    Serial.println(devListHeapJson);
}

#ifdef UDP_ENABLED
AsyncUDP asyncUdp;

void asyncUdpInit() {
    if (asyncUdp.listenMulticast(IPAddress(239, 255, 255, 255), 4210)) {
        asyncUdp.onPacket([](AsyncUDPPacket packet) {
            // Serial.print("UDP Packet Type: ");
            // Serial.println(packet.isBroadcast() ? "Broadcast" : packet.isMulticast() ? "Multicast" : "Unicast");
            // Serial.print("From: ");
            // Serial.print(packet.remoteIP());
            // Serial.print(":");
            // Serial.println(packet.remotePort());
            // Serial.print("To: ");
            // Serial.print(packet.localIP());
            // Serial.print(":");
            // Serial.println(packet.localPort());
            // Serial.print(", Length: ");
            // Serial.print(packet.length());
            // Serial.print(", Data: ");
            // Serial.write(packet.data(), packet.length());

            String data = uint8tToString(packet.data(), packet.length());
            SerialPrint("i", F("UDP"), "Udp packet received, IP: " + packet.remoteIP().toString() + ":" + String(packet.remotePort()));
            Serial.println(data);

            if (udpPacketValidation(data)) {
                SerialPrint("i", F("UDP"), F("Device presentation received"));
                jsonMergeArrays(devListHeapJson, data);
                Serial.println(devListHeapJson);
            } else {
                SerialPrint("E", F("UDP"), F("Udp packet invalid"));
            }

            // reply to the client
            // String ip = WiFi.localIP().toString();
            // asyncUdp.broadcastTo(ip.c_str(), packet.remotePort());
            // packet.printf(ip.c_str(), packet.length());
        });
    }

    //будем отправлять каждые 10 секунд презентацию данного устройства
    ts.add(
        UDP, 10000, [&](void*) {
            SerialPrint("i", F("UDP"), F("Broadcast device presentation"));
            asyncUdp.broadcastTo(devListHeapJson.c_str(), 4210);
            // asyncUdp.broadcast("test");
            // asyncUdp.print("Hello Server!");
        },
        nullptr, true);

    SerialPrint("I", F("UDP"), F("Udp Init"));
}

bool udpPacketValidation(String& data) {
    if (data.indexOf("devicelist") != -1) {
        return true;
    } else {
        return false;
    }
}

void jsonMergeArrays(String& arr1, String& arr2) {
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, arr1);
    if (error) {
        SerialPrint("E", F("UDP"), error.f_str());
    }
    JsonArray jarr = doc.as<JsonArray>();
    arr2.replace("[", "");
    arr2.replace("]", "");
    String incIP = jsonReadStr(arr2, "ip");

    Serial.println("incIP " + incIP);
    bool ipExistInList = false;
    for (JsonVariant value : jarr) {
        String locIP = value["ip"].as<String>();
        Serial.println("locIP " + locIP);
        if (locIP == incIP) {
            ipExistInList = true;
            break;
        } else {
            ipExistInList = false;
        }
    }
    if (!ipExistInList) {
        arr1.replace("]", "");
        arr1 = arr1 + "," + arr2 + "]";
    }
}

// void udpPacketParse(String& data) {
//     if (data.indexOf("scen:") != -1) {
//         data = deleteBeforeDelimiter(data, ":");
//         writeFile(String(DEVICE_SCENARIO_FILE), data);
//         loadScenario();
//     } else if (data.indexOf("event:") != -1) {
//         data = deleteBeforeDelimiter(data, ":");
//         eventBuf += data;
//     }
// }

String uint8tToString(uint8_t* data, size_t len) {
    String ret;
    while (len--) {
        ret += (char)*data++;
    }
    return ret;
}
#endif