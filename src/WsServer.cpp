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
            if (num >= 3) {
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
                sendFileToWsByFrames("/layout.json", "layout", "", num, WEB_SOCKETS_FRAME_SIZE);
            }

            //отвечаем на запрос параметров
            if (headerStr == "/params|") {
                String params = "{}";
                // jsonWriteStr(params, "params_", "");  //метка для парсинга
                for (std::list<IoTItem*>::iterator it = IoTItems.begin(); it != IoTItems.end(); ++it) {
                    if ((*it)->getSubtype() != "Loging") {
                        if ((*it)->iAmLocal) jsonWriteStr(params, (*it)->getID(), (*it)->getValue());
                    }
                }
                sendStringToWs("params", params, num);
            }

            //отвечаем на запрос графиков
            if (headerStr == "/charts|") {
                //обращение к логированию из ядра
                //отправка данных графиков только в выбранный сокет
                for (std::list<IoTItem*>::iterator it = IoTItems.begin(); it != IoTItems.end(); ++it) {
                    //сбрасываем даты графиков
                    // if ((*it)->getID().endsWith("-date")) {
                    //    (*it)->setTodayDate();
                    //}
                    if ((*it)->getSubtype() == "Loging" || "LogingDaily") {
                        (*it)->setPublishDestination(TO_WS, num);
                        (*it)->publishValue();
                    }
                }
            }

            //----------------------------------------------------------------------//
            // Страница веб интерфейса configutation
            //----------------------------------------------------------------------//

            //отвечаем данными на запрос страницы
            if (headerStr == "/config|") {
                sendFileToWsByFrames("/items.json", "itemsj", "", num, WEB_SOCKETS_FRAME_SIZE);
                sendFileToWsByFrames("/widgets.json", "widget", "", num, WEB_SOCKETS_FRAME_SIZE);
                sendFileToWsByFrames("/config.json", "config", "", num, WEB_SOCKETS_FRAME_SIZE);
                sendFileToWsByFrames("/scenario.txt", "scenar", "", num, WEB_SOCKETS_FRAME_SIZE);
                sendStringToWs("settin", settingsFlashJson, num);
            }

            //обработка кнопки сохранить
            if (headerStr == "/gifnoc|") {
                writeFileUint8tByFrames("config.json", payload, length, headerLenth, 256);
            }
            if (headerStr == "/tuoyal|") {
                writeFileUint8tByFrames("layout.json", payload, length, headerLenth, 256);
            }
            if (headerStr == "/oiranecs|") {
                writeFileUint8tByFrames("scenario.txt", payload, length, headerLenth, 256);
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
                sendStringToWs("settin", settingsFlashJson, num);
                sendStringToWs("ssidli", ssidListHeapJson, num);
                sendStringToWs("errors", errorsHeapJson, num);
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
                sendStringToWs("devlis", devListHeapJson, num);
            }

            //----------------------------------------------------------------------//
            // Страница веб интерфейса system
            //----------------------------------------------------------------------//

            //отвечаем данными на запрос страницы
            if (headerStr == "/system|") {
                sendStringToWs("errors", errorsHeapJson, num);
                sendStringToWs("settin", settingsFlashJson, num);
            }

            //----------------------------------------------------------------------//
            // Страница веб интерфейса dev
            //----------------------------------------------------------------------//
            if (headerStr == "/dev|") {
                // standWebSocket.sendTXT(num, errorsHeapJson);
                // standWebSocket.sendTXT(num, settingsFlashJson);
                // sendFileToWs("/config.json", num, 1024);
                // sendFileToWs("/items.json", num, 1024);
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

            if (headerStr == "/test|") {
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
    // String path = mqttRootDevice + "/" + topic;
    // String json = "{}";
    // jsonWriteStr(json, "status", data);
    // jsonWriteStr(json, "topic", path);
    // standWebSocket.broadcastTXT(json);
}

//публикация статус сообщений
// void publishChartWs2(int num, String& data) {
//    bool ok = false;
//    if (num == -1) {
//        ok = standWebSocket.broadcastTXT(data);
//    } else {
//        ok = standWebSocket.sendTXT(num, data);
//    }
//    if (ok) {
//        SerialPrint(F("i"), F("WS"), F("sent sucsess"));
//    } else {
//        SerialPrint(F("E"), F("WS"), F("sent error"));
//    }
//}

void publishChartWs(int num, String& path) {
    sendFileToWs(path, num, 1000);
}

//данные которые мы отправляем в сокеты переодически
void periodicWsSend() {
    sendStringToWs("ssidli", ssidListHeapJson, -1);
    sendStringToWs("errors", errorsHeapJson, -1);
    sendStringToWs("devlis", devListHeapJson, -1);
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
void sendFileToWs(String filename, int num, size_t frameSize) {
    // String st = "/st" + String(filename);
    // if (num == -1) {
    //     standWebSocket.broadcastTXT(st);
    // } else {
    //     standWebSocket.sendTXT(num, st);
    // }
    //
    // String path = filepath(filename);
    // auto file = FileFS.open(path, "r");
    // if (!file) {
    //    SerialPrint(F("E"), F("FS"), F("reed file error"));
    //    return;
    //}
    // size_t fileSize = file.size();
    // SerialPrint(F("i"), F("FS"), "Send file '" + String(filename) + "', file size: " + String(fileSize));
    // uint8_t payload[frameSize];
    // int countRead = file.read(payload, sizeof(payload));
    // while (countRead > 0) {
    //    if (num == -1) {
    //        standWebSocket.broadcastBIN(payload, countRead);
    //    } else {
    //        standWebSocket.sendBIN(num, payload, countRead);
    //    }
    //    countRead = file.read(payload, sizeof(payload));
    //}
    // file.close();
    // String end = "/end" + String(filename);
    // if (num == -1) {
    //    standWebSocket.broadcastTXT(end);
    //} else {
    //    standWebSocket.sendTXT(num, end);
    //}
}

//посылка данных из string
// void sendStringToWs(const String& msg, uint8_t num, String name) {
//    String st = "/st" + String(name);
//    standWebSocket.sendTXT(num, st);
//    size_t size = msg.length();
//    char dataArray[size];
//    msg.toCharArray(dataArray, size);
//    standWebSocket.sendBIN(num, (uint8_t*)dataArray, size);
//    String end = "/end" + String(name);
//    standWebSocket.sendTXT(num, end);
//}

//особая функция отправки графиков в веб
void publishChartToWs(String filename, int num, size_t frameSize, int maxCount, String id) {
    // String json;
    // jsonWriteStr(json, "topic", mqttRootDevice + "/" + id);
    // jsonWriteInt(json, "maxCount", maxCount);
    //
    // String st = "/st/chart.json|" + json;
    // if (num == -1) {
    //    standWebSocket.broadcastTXT(st);
    //} else {
    //    standWebSocket.sendTXT(num, st);
    //}
    // String path = filepath(filename);
    // auto file = FileFS.open(path, "r");
    // if (!file) {
    //    SerialPrint(F("E"), F("FS"), F("reed file error"));
    //    return;
    //}
    // size_t fileSize = file.size();
    // SerialPrint(F("i"), F("FS"), "Send file '" + String(filename) + "', file size: " + String(fileSize));
    // uint8_t payload[frameSize];
    // int countRead = file.read(payload, sizeof(payload));
    // while (countRead > 0) {
    //    if (num == -1) {
    //        standWebSocket.broadcastBIN(payload, countRead);
    //    } else {
    //        standWebSocket.sendBIN(num, payload, countRead);
    //    }
    //    countRead = file.read(payload, sizeof(payload));
    //}
    // file.close();
    // String end = "/end/chart.json|" + json;
    // if (num == -1) {
    //    standWebSocket.broadcastTXT(end);
    //} else {
    //    standWebSocket.sendTXT(num, end);
    //}
}

void sendFileToWsByFrames(const String& filename, const String& header, const String& json, uint8_t client_id, size_t frameSize) {
    if (header.length() != 6) {
        SerialPrint("E", "FS", F("wrong header size"));
        return;
    }

    auto path = filepath(filename);
    auto file = FileFS.open(path, "r");
    if (!file) {
        SerialPrint("E", "FS", F("reed file error"));
        return;
    }

    size_t totalSize = file.size();
    // Serial.println("Send file '" + String(filename) + "', file size: " + String(totalSize));

    char buf[32];
    sprintf(buf, "%04d", json.length() + 12);
    String data = header + "|" + String(buf) + "|" + json;

    size_t headerSize = data.length();
    auto frameBuf = new uint8_t[frameSize];
    size_t maxPayloadSize = frameSize - headerSize;
    uint8_t* payloadBuf = nullptr;

    int i = 0;
    while (file.available()) {
        if (i == 0) {
            data.toCharArray((char*)frameBuf, frameSize);
            payloadBuf = &frameBuf[headerSize];
        } else {
            maxPayloadSize = frameSize;
            headerSize = 0;
            payloadBuf = &frameBuf[0];
        }

        size_t payloadSize = file.read(payloadBuf, maxPayloadSize);
        if (payloadSize) {
            size_t size = headerSize + payloadSize;

            bool fin = false;
            if (size == frameSize) {
                fin = false;
            } else {
                fin = true;
            }

            bool continuation = false;
            if (i == 0) {
                continuation = false;
            } else {
                continuation = true;
            }

            // Serial.println(String(i) + ") fr sz: " + String(size) + " fin: " + String(fin) + " cnt: " + String(continuation));
            standWebSocket.sendBIN(client_id, frameBuf, size, fin, continuation);
        }
        i++;
    }
    payloadBuf = &frameBuf[0];
    delete[] payloadBuf;
    file.close();
}

void sendStringToWs(const String& header, String& payload, int client_id) {
    if (header.length() != 6) {
        SerialPrint("E", "FS", F("wrong header size"));
        return;
    }

    String msg = header + "|0012|" + payload;
    size_t totalSize = msg.length();
    // Serial.println("Send string '" + header + "', str size: " + String(totalSize));

    char dataArray[totalSize];
    msg.toCharArray(dataArray, totalSize + 1);
    if (client_id == -1) {
        standWebSocket.broadcastBIN((uint8_t*)dataArray, totalSize);
    } else {
        standWebSocket.sendBIN(client_id, (uint8_t*)dataArray, totalSize);
    }
}

// void sendFileToWsByFrames(const String& filename, const String& header, const String& json, uint8_t client_id, size_t frameSize) {
//     if (header.length() != 6) {
//         SerialPrint("E", "FS", F("wrong header size"));
//         return;
//     }
//     // откроем файл
//     auto path = filepath(filename);
//     auto file = FileFS.open(path, "r");
//     if (!file) {
//         SerialPrint("E", "FS", F("reed file error"));
//         return;
//     }
//
//     size_t totalSize = file.size();
//     SerialPrint("I", "FS", "Send file '" + String(filename) + "', file size: " + String(totalSize));
//
//     char buf[32];
//     sprintf(buf, "%04d", json.length() + 12);
//
//     String data = header + "|" + String(buf) + "|" + json;
//
//     // размер заголовка
//     auto headerSize = data.length();
//     // выделим буфер размером с фрейм
//     auto frameBuf = new uint8_t[frameSize];
//     // заголовок у нас не меняется, запием его в начало буфера
//     data.toCharArray((char*)frameBuf, frameSize);
//     // указатель на начало полезной нагрузки
//     auto payloadBuf = &frameBuf[headerSize];
//     // и сколько осталось места для нее
//     auto maxPayloadSize = frameSize - headerSize;
//     int i = 0;
//     while (file.available()) {
//         // прочитаем кусок в буфер
//         size_t payloadSize = file.read(payloadBuf, maxPayloadSize);
//         if (payloadSize) {
//             size_t size = headerSize + payloadSize;
//
//             bool fin = false;
//             if (size == frameSize) {
//                 fin = false;
//             } else {
//                 fin = true;
//             }
//
//             bool continuation = false;
//             if (i == 0) {
//                 continuation = false;
//             } else {
//                 continuation = true;
//             }
//
//             SerialPrint("I", "FS", String(i) + ") sz: " + String(size) + " fin: " + String(fin) + " cnt: " + String(continuation));
//             standWebSocket.sendBIN(client_id, frameBuf, size, fin, continuation);
//         }
//         i++;
//     }
// }

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
