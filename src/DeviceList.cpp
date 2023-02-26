#include "DeviceList.h"

const String getThisDevice() {
    String thisDevice = "{}";
    jsonWriteStr_(thisDevice, F("devicelist_"), "");                              //метка для парсинга
    jsonWriteStr_(thisDevice, F("wg"), jsonReadStr(settingsFlashJson, F("wg")));  //рабочая группа
    jsonWriteStr_(thisDevice, F("ip"), jsonReadStr(settingsFlashJson, F("ip")));
    jsonWriteStr_(thisDevice, F("id"), jsonReadStr(settingsFlashJson, F("id")));
    jsonWriteStr_(thisDevice, F("name"), jsonReadStr(settingsFlashJson, F("name")));
    thisDevice = "[" + thisDevice + "]";
    return thisDevice;
}

void addThisDeviceToList() {
    devListHeapJson = getThisDevice();
    SerialPrint("i", "List", "Add this dev to list");
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
            String remoteWorkgroup = "";
            data.replace("[", "");
            data.replace("]", "");
            if (jsonRead(data, F("wg"), remoteWorkgroup)) {    // проверяем чтоб полученный формат был Json и заодно вытягиваем имя группы
                String loacalWorkgroup = "";
                jsonRead(settingsFlashJson, F("wg"), loacalWorkgroup);  
                if (remoteWorkgroup == loacalWorkgroup) {
                    SerialPrint("i", F("UDP"), "IP: " + packet.remoteIP().toString() + ":" + String(packet.remotePort()));
                    jsonMergeArrays(devListHeapJson, data);
                }
            } else {
                SerialPrint("E", F("UDP"), F("Udp packet invalid"));
            }
            // reply to the client
            // String ip = WiFi.localIP().toString();
            // asyncUdp.broadcastTo(ip.c_str(), packet.remotePort());
            // packet.printf(ip.c_str(), packet.length());
        });
    }

    //будем отправлять каждые 60 секунд презентацию данного устройства
    ts.add(
        UDP, 60000, [&](void*) {  // UDPP
            if (isNetworkActive()) {
                SerialPrint("i", F("UDP"), F("Broadcast device presentation"));
                asyncUdp.broadcastTo(getThisDevice().c_str(), 4210);
                // asyncUdp.broadcast("test");
                // asyncUdp.print("Hello Server!");
            }
        },
        nullptr, true);

    SerialPrint("i", F("UDP"), F("Udp Init"));
}

void jsonMergeArrays(String& existJson, String& incJson) {
    DynamicJsonDocument incJsonDoc(1024);
    DeserializationError incJsonError = deserializeJson(incJsonDoc, incJson);
    if (incJsonError) {
        SerialPrint("E", F("UDP"), "Invailed json in incomming udp packet " + String(incJsonError.f_str()));
        jsonErrorDetected();
        return;
    }

    DynamicJsonDocument existJsonDoc(1024);
    DeserializationError existJsonError = deserializeJson(existJsonDoc, existJson);
    if (existJsonError) {
        SerialPrint("E", F("UDP"), "Invailed json in existing udp dev list " + String(incJsonError.f_str()));
        jsonErrorDetected();
        return;
    }
    JsonArray existJsonArr = existJsonDoc.as<JsonArray>();

    incJson.replace("[", "");
    incJson.replace("]", "");
    String incIP = jsonReadStr(incJson, "ip");
    String outArr = "[";
    bool ipExistInList = false;
    int i = 0;
    for (JsonVariant value : existJsonArr) {
        String locIP = value["ip"].as<String>();
        if (locIP == incIP) {
            if (i == 0) {
                outArr += incJson;
            } else {
                outArr += ("," + incJson);
            }
            ipExistInList = true;
        } else {
            if (i == 0) {
                outArr += value.as<String>();
            } else {
                outArr += ("," + value.as<String>());
            }
        }
        i++;
    }
    if (!ipExistInList) {
        outArr += "," + incJson;
    }
    outArr = outArr + "]";
    existJson = outArr;
}

String uint8tToString(uint8_t* data, size_t len) {
    String ret;
    while (len--) {
        ret += (char)*data++;
    }
    return ret;
}
#endif