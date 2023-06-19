#include "DeviceList.h"

const String getThisDevice() {
    String thisDevice = "{}";
    // jsonWriteStr_(thisDevice, F("devicelist_"), "");                            // метка для парсинга - она не нужна теперь
    jsonWriteStr_(thisDevice, F("wg"), jsonReadStr(settingsFlashJson, F("wg")));  // рабочая группа
    jsonWriteStr_(thisDevice, F("ip"), jsonReadStr(settingsFlashJson, F("ip")));
    jsonWriteStr_(thisDevice, F("id"), jsonReadStr(settingsFlashJson, F("id")));
    jsonWriteStr_(thisDevice, F("name"), jsonReadStr(settingsFlashJson, F("name")));
    // приложение svelte хранит свой список и в нем поддерживает корректные статусы, а при получении этого списка
    // лишь изменяет в своем те элементы массива которые добавились новые, не трогая остальные
    jsonWriteBool_(thisDevice, F("status"), false);
    jsonWriteInt_(thisDevice, F("fv"), FIRMWARE_VERSION);
    thisDevice = "[" + thisDevice + "]";
    return thisDevice;
}

void addThisDeviceToList() {
    if (jsonReadInt(settingsFlashJson, F("udps")) != 0) {
        // если включен автопоиск то записываем это устройство в оперативную память
        devListHeapJson = getThisDevice();
    } else {
        // если выключен автопоиск то записываем это устройство в флешь память
        // если файла не было никогда
        String list = readFile("/devlist.json", 2048);
        if (list == "failed") {
            writeFile("/devlist.json", getThisDevice());
        } else {
            // если файл был то перепишем в нем первое устройство - to do
        }
    }

    SerialPrint("i", F("List"), F("This dev added to list"));
}

#ifdef UDP_ENABLED
AsyncUDP asyncUdp;

void udpListningInit() {
    if (asyncUdp.listenMulticast(IPAddress(239, 255, 255, 255), 4210)) {
        asyncUdp.onPacket([](AsyncUDPPacket packet) {
            // если был включен автоматический поиск устройств то начнем запись в оперативную память
            if (jsonReadInt(settingsFlashJson, F("udps")) != 0) {
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
                // String data = {packet.data(), packet.length()}; // для ESP32 подходит как замена uint8tToString, но 8266 не переваривает

                String data = uint8tToString(packet.data(), packet.length());
                String remoteWorkgroup = "";
                data.replace("[", "");
                data.replace("]", "");
                if (jsonRead(data, F("wg"), remoteWorkgroup)) {  // проверяем чтоб полученный формат был Json и заодно вытягиваем имя группы
                    String loacalWorkgroup = "";
                    jsonRead(settingsFlashJson, F("wg"), loacalWorkgroup);
                    if (remoteWorkgroup == loacalWorkgroup) {
                        SerialPrint("i", F("UDP"), "IP: " + packet.remoteIP().toString() + ":" + String(packet.remotePort()));
                        jsonMergeArrays(devListHeapJson, data);
                        // эксперементальный вариант отправки нового списка сразу по приходу
                        // sendStringToWs("devlis", devListHeapJson, -1);
                    }
                } else {
                    SerialPrint("E", F("UDP"), F("Udp packet invalid"));
                }

                // reply to the client
                // String ip = WiFi.localIP().toString();
                // asyncUdp.broadcastTo(ip.c_str(), packet.remotePort());
                // packet.printf(ip.c_str(), packet.length());

            } else {
                devListHeapJson = "";
            }
        });
    }

    SerialPrint("i", F("UDP"), F("Udp listning inited"));
}

void udpBroadcastInit() {
    // будем отправлять каждые 60 секунд презентацию данного устройства
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

    SerialPrint("i", F("UDP"), F("Udp broadcast inited"));
}

void jsonMergeArrays(String& existJson, String& incJson) {
    DynamicJsonDocument incJsonDoc(4096);
    DeserializationError incJsonError = deserializeJson(incJsonDoc, incJson);
    // if (incJsonError) {  // upd: devlist заведомо верный, зачем проверять еще раз?
    //     SerialPrint("E", F("UDP"), "Invailed json in incomming udp packet " + String(incJsonError.f_str()));
    //     jsonErrorDetected();
    //     return;
    // }

    DynamicJsonDocument existJsonDoc(4096);
    DeserializationError existJsonError = deserializeJson(existJsonDoc, existJson);
    // if (existJsonError) {    // upd: полученный json уже проверен на целостность
    //     SerialPrint("E", F("UDP"), "Invailed json in existing udp dev list " + String(incJsonError.f_str()));
    //     jsonErrorDetected();
    //     return;
    // }
    JsonArray existJsonArr = existJsonDoc.as<JsonArray>();

    // incJson.replace("[", "");    // upd: уже исключены символы ранее при получении пакета
    // incJson.replace("]", "");
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