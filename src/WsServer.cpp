#include "WsServer.h"
#include "classes/IoTScenario.h"
extern IoTScenario iotScen;

#ifdef STANDARD_WEB_SOCKETS
void standWebSocketsInit() {
    standWebSocket.begin();
    standWebSocket.onEvent(webSocketEvent);
    SerialPrint("i", "WS", "WS server initialized");
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    switch (type) {
        case WStype_ERROR: {
            Serial.printf("[%u] Error!\n", num);
        } break;

        case WStype_DISCONNECTED: {
            Serial.printf("[%u] Disconnected!\n", num);
        } break;

        case WStype_CONNECTED: {
            // IPAddress ip = standWebSocket.remoteIP(num);
            SerialPrint("i", "WS " + String(num), "WS client connected");
            if (num > 3) {
                SerialPrint("E", "WS", "Too many clients, connection closed!!!");
                jsonWriteInt(errorsHeapJson, "wse1", 1);
                standWebSocket.close();
                standWebSocketsInit();
            }
            // Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
            // standWebSocket.sendTXT(num, "Connected");
        } break;

        case WStype_TEXT: {
            bool endOfHeaderFound = false;
            size_t maxAllowedHeaderSize = 15;  //максимальное количество символов заголовка
            size_t headerLenth = 0;
            String headerStr;
            for (size_t i = 0; i <= maxAllowedHeaderSize; i++) {
                headerLenth++;
                char s = (char)payload[i];
                headerStr += s;
                if (s == '|') {
                    endOfHeaderFound = true;
                    break;
                }
            }
            if (!endOfHeaderFound) {
                SerialPrint("E", "WS " + String(num), "Package without header");
            }

            //----------------------------------------------------------------------//
            // Страница веб интерфейса dashboard
            //----------------------------------------------------------------------//

            //отвечаем данными на запрос страницы
            if (headerStr == "/|") {
                sendFileToWs("/layout.json", num, 1024);
            }

            //отвечаем на запрос параметров
            if (headerStr == "/params|") {
                String json = "{}";
                jsonWriteStr(json, "params", "");
                for (std::list<IoTItem*>::iterator it = IoTItems.begin(); it != IoTItems.end(); ++it) {
                    if ((*it)->getSubtype() != "Loging") {
                        if ((*it)->iAmLocal) jsonWriteStr(json, (*it)->getID(), (*it)->getValue());
                    }
                }

                standWebSocket.sendTXT(num, json);

                //отправка данных графиков только в выбранный сокет
                for (std::list<IoTItem*>::iterator it = IoTItems.begin(); it != IoTItems.end(); ++it) {
                    //сбрасываем даты графиков
                    // if ((*it)->getID().endsWith("-date")) {
                    //    (*it)->setTodayDate();
                    //}
                    if ((*it)->getSubtype() == "Loging") {
                        (*it)->setPublishDestination(2, num);
                        (*it)->publishValue();
                    }
                }
            }

            //----------------------------------------------------------------------//
            // Страница веб интерфейса configutation
            //----------------------------------------------------------------------//

            //отвечаем данными на запрос страницы
            if (headerStr == "/config|") {
                sendFileToWs("/items.json", num, 1024);
                sendFileToWs("/widgets.json", num, 1024);
                sendFileToWs("/config.json", num, 1024);
                sendFileToWs("/scenario.json", num, 1024);
                standWebSocket.sendTXT(num, settingsFlashJson);
            }

            //обработка кнопки сохранить
            if (headerStr == "/gifnoc|") {
                writeFileUint8tByFrames("config.json", payload, length, headerLenth, 256);
            }
            if (headerStr == "/tuoyal|") {
                writeFileUint8tByFrames("layout.json", payload, length, headerLenth, 256);
            }
            if (headerStr == "/oiranecs|") {
                writeFileUint8tByFrames("scenario.json", payload, length, headerLenth, 256);

                String strFromFile;
                File myfile = seekFile("/scenario.json");
                if (myfile.available()) {
                    strFromFile = myfile.readString();

                    strFromFile.replace("{\"scen\":\"", "");
                    strFromFile.replace("\\n", "\n");
                    strFromFile.replace("\\\"", "\"");
                    strFromFile.replace(";", " ");
                    strFromFile.replace("\\t", " ");
                    strFromFile.remove(strFromFile.length() - 2, 2);
                }
                myfile.close();

                writeFile("/scenario.txt", strFromFile);

                clearConfigure();
                configure("/config.json");
                iotScen.loadScenario("/scenario.txt");
                // создаем событие завершения конфигурирования для возможности выполнения блока кода при загрузке
                IoTItems.push_back((IoTItem*)new externalVariable("{\"id\":\"onStart\",\"val\":1,\"int\":60}"));
                generateEvent("onStart", "");
            }

            //----------------------------------------------------------------------//
            // Страница веб интерфейса connection
            //----------------------------------------------------------------------//

            //отвечаем данными на запрос страницы
            if (headerStr == "/connection|") {
                standWebSocket.sendTXT(num, settingsFlashJson);
                standWebSocket.sendTXT(num, ssidListHeapJson);
                standWebSocket.sendTXT(num, errorsHeapJson);
                // запуск асинхронного сканирования wifi сетей при переходе на страницу соединений
                // RouterFind(jsonReadStr(settingsFlashJson, F("routerssid")));
            }

            //обработка кнопки сохранить настройки wifi
            if (headerStr == "/sgnittes|") {
                writeUint8tToString(payload, length, headerLenth, settingsFlashJson);
                writeFileUint8tByFrames("settings.json", payload, length, headerLenth, 256);
                standWebSocket.sendTXT(num, errorsHeapJson);
                addThisDeviceToList();
            }

            //обработка кнопки сохранить настройки mqtt
            if (headerStr == "/mqtt|") {
                standWebSocket.sendTXT(num, settingsFlashJson);  //отправляем в ответ новые полученные настройки
                handleMqttStatus(false, 8);                      //меняем статус на неопределенный
                mqttReconnect();                                 //начинаем переподключение
                standWebSocket.sendTXT(num, errorsHeapJson);     //отправляем что статус неопределен
                standWebSocket.sendTXT(num, ssidListHeapJson);
            }

            //запуск асинхронного сканирования wifi сетей при нажатии выпадающего списка
            if (headerStr == "/scan|") {
                RouterFind(jsonReadStr(settingsFlashJson, F("routerssid")));
                standWebSocket.sendTXT(num, ssidListHeapJson);
            }

            //----------------------------------------------------------------------//
            // Страница веб интерфейса list
            //----------------------------------------------------------------------//

            //отвечаем данными на запрос страницы
            if (headerStr == "/list|") {
                standWebSocket.sendTXT(num, devListHeapJson);
            }

            //----------------------------------------------------------------------//
            // Страница веб интерфейса system
            //----------------------------------------------------------------------//

            //отвечаем данными на запрос страницы
            if (headerStr == "/system|") {
                standWebSocket.sendTXT(num, errorsHeapJson);
                standWebSocket.sendTXT(num, settingsFlashJson);
            }

            //----------------------------------------------------------------------//
            // Страница веб интерфейса dev
            //----------------------------------------------------------------------//
            if (headerStr == "/dev|") {
                standWebSocket.sendTXT(num, errorsHeapJson);
                sendFileToWs("/layout.json", num, 1024);
                standWebSocket.sendTXT(num, settingsFlashJson);
                sendFileToWs("/config.json", num, 1024);
                sendFileToWs("/items.json", num, 1024);
                String json = getParamsJson();
                standWebSocket.sendTXT(num, json);
            }

            //----------------------------------------------------------------------//
            // отдельные команды веб интерфейса
            //----------------------------------------------------------------------//

            //переписать любое поле в errors json
            if (headerStr == "/rorre|") {
                writeUint8tValueToJsonString(payload, length, headerLenth, errorsHeapJson);
            }

            //команда перезагрузки esp
            if (headerStr == "/reboot|") {
                ESP.restart();
            }

            //команда очистки всех логов esp
            if (headerStr == "/clean|") {
                cleanLogs();
            }

            //команда обновления прошивки esp
            if (headerStr == "/update|") {
                upgrade_firmware(3);
            }

            //Прием команд control c dashboard
            if (headerStr == "/control|") {
                String msg;
                writeUint8tToString(payload, length, headerLenth, msg);
                String key = selectFromMarkerToMarker(msg, "/", 0);
                String value = selectFromMarkerToMarker(msg, "/", 1);
                generateOrder(key, value);
                SerialPrint("i", F("=>WS"), "Msg from svelte web, WS No: " + String(num) + ", msg: " + msg);
            }
        } break;

        case WStype_BIN: {
            Serial.printf("[%u] get binary length: %u\n", num, length);
            // hexdump(payload, length);
            // standWebSocket.sendBIN(num, payload, length);
        } break;

        case WStype_FRAGMENT_TEXT_START: {
            Serial.printf("[%u] fragment test start: %u\n", num, length);
        } break;

        case WStype_FRAGMENT_BIN_START: {
            Serial.printf("[%u] fragment bin start: %u\n", num, length);
        } break;

        case WStype_FRAGMENT: {
            Serial.printf("[%u] fragment: %u\n", num, length);
        } break;

        case WStype_FRAGMENT_FIN: {
            Serial.printf("[%u] fragment finish: %u\n", num, length);
        } break;

        case WStype_PING: {
            Serial.printf("[%u] ping: %u\n", num, length);
        } break;

        case WStype_PONG: {
            Serial.printf("[%u] pong: %u\n", num, length);
        } break;

        default: {
            Serial.printf("[%u] not recognized: %u\n", num, length);
        } break;
    }
}

//публикация статус сообщений (недостаток в том что делаем бродкаст всем клиентам поднятым в свелте!!!)
void publishStatusWs(const String& topic, const String& data) {
    String path = mqttRootDevice + "/" + topic;
    String json = "{}";
    jsonWriteStr(json, "status", data);
    jsonWriteStr(json, "topic", path);
    standWebSocket.broadcastTXT(json);
}

//публикация статус сообщений
void publishChartWs(int num, String& data) {
    bool ok = false;
    if (num == -1) {
        ok = standWebSocket.broadcastTXT(data);
    } else {
        ok = standWebSocket.sendTXT(num, data);
    }
    if (ok) {
        SerialPrint(F("i"), F("WS"), F("sent sucsess"));
    } else {
        SerialPrint(F("E"), F("WS"), F("sent error"));
    }
}

//данные которые мы отправляем в сокеты переодически
void periodicWsSend() {
    standWebSocket.broadcastTXT(devListHeapJson);
    standWebSocket.broadcastTXT(ssidListHeapJson);
    standWebSocket.broadcastTXT(errorsHeapJson);
}

#ifdef ESP32
void hexdump(const void* mem, uint32_t len, uint8_t cols = 16) {
    const uint8_t* src = (const uint8_t*)mem;
    Serial.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);
    for (uint32_t i = 0; i < len; i++) {
        if (i % cols == 0) {
            Serial.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
        }
        Serial.printf("%02X ", *src);
        src++;
    }
    Serial.printf("\n");
}
#endif
#endif

//посылка данных из файла в бинарном виде
void sendFileToWs(const char* filename, uint8_t num, size_t frameSize) {
    String st = "/st" + String(filename);
    standWebSocket.sendTXT(num, st);
    String path = filepath(filename);
    auto file = FileFS.open(path, "r");
    if (!file) {
        SerialPrint(F("E"), F("FS"), F("reed file error"));
        return;
    }
    size_t fileSize = file.size();
    SerialPrint(F("i"), F("FS"), "Send file '" + String(filename) + "', file size: " + String(fileSize));
    uint8_t payload[frameSize];
    int countRead = file.read(payload, sizeof(payload));
    while (countRead > 0) {
        standWebSocket.sendBIN(num, payload, countRead);
        countRead = file.read(payload, sizeof(payload));
    }
    file.close();
    String end = "/end" + String(filename);
    standWebSocket.sendTXT(num, end);
}

//посылка данных из string
void sendStringToWs(const String& msg, uint8_t num, String name) {
    String st = "/st" + String(name);
    standWebSocket.sendTXT(num, st);
    size_t size = msg.length();
    char dataArray[size];
    msg.toCharArray(dataArray, size);
    standWebSocket.sendBIN(num, (uint8_t*)dataArray, size);
    String end = "/end" + String(name);
    standWebSocket.sendTXT(num, end);
}

// void sendMark(const char* filename, const char* mark, uint8_t num) {
//     char outChar[strlen(filename) + strlen(mark) + 1];
//     strcpy(outChar, mark);
//     strcat(outChar, filename);
//     size_t size = strlen(outChar);
//     uint8_t outUint[size];
//     for (size_t i = 0; i < size; i++) {
//         outUint[i] = uint8_t(outChar[i]);
//     }
//     standWebSocket.sendBIN(num, outUint, sizeof(outUint));
// }

//посылка данных из файла в string
// void sendFileToWs3(const String& filename, uint8_t num) {
//    standWebSocket.sendTXT(num, "/st" + filename);
//    size_t ws_buffer = 512;
//    String path = filepath(filename);
//    auto file = FileFS.open(path, "r");
//    if (!file) {
//        SerialPrint(F("E"), F("FS"), F("reed file error"));
//    }
//    size_t fileSize = file.size();
//    SerialPrint(F("i"), F("WS"), "Send file '" + filename + "', file size: " + String(fileSize));
//    String ret;
//    char temp[ws_buffer + 1];
//    int countRead = file.readBytes(temp, sizeof(temp) - 1);
//    while (countRead > 0) {
//        temp[countRead] = 0;
//        ret = temp;
//        standWebSocket.sendTXT(num, ret);
//        countRead = file.readBytes(temp, sizeof(temp) - 1);
//    }
//    standWebSocket.sendTXT(num, "/end" + filename);
//}

//посылка данных из файла в char
// void sendFileToWs4(const String& filename, uint8_t num) {
//    standWebSocket.sendTXT(num, "/st" + filename);
//    size_t ws_buffer = 512;
//    String path = filepath(filename);
//    auto file = FileFS.open(path, "r");
//    if (!file) {
//        SerialPrint(F("E"), F("FS"), F("reed file error"));
//    }
//    size_t fileSize = file.size();
//    SerialPrint(F("i"), F("WS"), "Send file '" + filename + "', file size: " + String(fileSize));
//    char temp[ws_buffer + 1];
//    int countRead = file.readBytes(temp, sizeof(temp) - 1);
//    while (countRead > 0) {
//        temp[countRead] = 0;
//        standWebSocket.sendTXT(num, temp, countRead);
//        countRead = file.readBytes(temp, sizeof(temp) - 1);
//    }
//}
