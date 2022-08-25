#include "MqttClient.h"
#include "NTP.h"

void mqttInit() {
    mqtt.setCallback(mqttCallback);
    ts.add(
        WIFI_MQTT_CONNECTION_CHECK, MQTT_RECONNECT_INTERVAL,
        [&](void*) {
            if (WiFi.status() == WL_CONNECTED) {
                SerialPrint("i", F("WIFI"), F("OK"));
                wifiUptimeCalc();
                if (mqtt.connected()) {
                    SerialPrint("i", F("MQTT"), "OK");
                    mqttUptimeCalc();
                    handleMqttStatus(false);

                    // setLedStatus(LED_OFF);
                } else {
                    SerialPrint("E", F("MQTT"), F("✖ Connection lost"));
                    handleMqttStatus(false);
                    mqttUptime = 0;
                    mqttConnect();
                }
            } else {
                SerialPrint("E", F("WIFI"), F("✖ Lost WiFi connection"));
                ts.remove(WIFI_MQTT_CONNECTION_CHECK);
                wifiUptime = 0;
                startAPMode();
            }
        },
        nullptr, true);
}

void mqttLoop() {
    if (!isNetworkActive() || !mqtt.connected()) {
        return;
    }
    mqtt.loop();
}

boolean mqttConnect() {
    getMqttData();
    bool res = false;
    if (mqttServer == "") {
        SerialPrint("E", "MQTT", F("mqttServer empty"));

        handleMqttStatus(true, 6);

        return res;
    }
    SerialPrint("i", "MQTT", "connection started");

    SerialPrint("i", "MQTT", "broker " + mqttServer + ":" + String(mqttPort, DEC));
    SerialPrint("i", "MQTT", "topic " + mqttRootDevice);

    // setLedStatus(LED_FAST);

    mqtt.setServer(mqttServer.c_str(), mqttPort);

    if (!mqtt.connected()) {
        bool connected = false;
        if (mqttUser != "" && mqttPass != "") {
            connected = mqtt.connect(chipId.c_str(), mqttUser.c_str(), mqttPass.c_str());
            SerialPrint("i", F("MQTT"), F("Go to connection with login and password"));
        } else if (mqttUser == "" && mqttPass == "") {
            connected = mqtt.connect(chipId.c_str());
            SerialPrint("i", F("MQTT"), F("Go to connection without login and password"));
        } else {
            SerialPrint("E", F("MQTT"), F("✖ Login or password missed"));
            handleMqttStatus(true, 7);
            return res;
        }

        if (mqtt.state() == 0) {
            SerialPrint("i", F("MQTT"), F("✔ connected"));
            handleMqttStatus(true);
            //   setLedStatus(LED_OFF);
            mqttSubscribe();
            res = true;
        } else {
            SerialPrint("E", F("MQTT"), "🡆 Could't connect, retry in " + String(MQTT_RECONNECT_INTERVAL / 1000) + "s");
            handleMqttStatus(true);
            //  setLedStatus(LED_FAST);
        }
    }
    return res;
}

void mqttDisconnect() {
    SerialPrint("i", F("MQTT"), F("disconnected"));
    mqtt.disconnect();
}

void mqttReconnect() {
    mqttDisconnect();
    mqttConnect();
}

void getMqttData() {
    mqttServer = jsonReadStr(settingsFlashJson, F("mqttServer"));
    mqttPort = jsonReadInt(settingsFlashJson, F("mqttPort"));
    mqttUser = jsonReadStr(settingsFlashJson, F("mqttUser"));
    mqttPass = jsonReadStr(settingsFlashJson, F("mqttPass"));
}

void mqttSubscribe() {
    SerialPrint("i", F("MQTT"), F("subscribed"));
    SerialPrint("i", F("MQTT"), mqttRootDevice);
    mqtt.subscribe(mqttPrefix.c_str());
    mqtt.subscribe((mqttRootDevice + "/+/control").c_str());
    mqtt.subscribe((mqttRootDevice + "/update").c_str());

    if (jsonReadBool(settingsFlashJson, "mqttin")) {
        mqtt.subscribe((mqttPrefix + "/+/+/event").c_str());
        mqtt.subscribe((mqttPrefix + "/+/+/order").c_str());
        mqtt.subscribe((mqttPrefix + "/+/+/info").c_str());
    }
}

void mqttCallback(char* topic, uint8_t* payload, size_t length) {
    String topicStr = String(topic);
    // SerialPrint("i", "=>MQTT", topicStr);
    String payloadStr;
    payloadStr.reserve(length + 1);
    for (size_t i = 0; i < length; i++) {
        payloadStr += (char)payload[i];
    }

    // SerialPrint("i", "=>MQTT", payloadStr);

    // SerialPrint("i", F("=>MQTT"), "Msg from iotmanager: " + topicStr);

    if (payloadStr.startsWith("HELLO")) {
        SerialPrint("i", F("MQTT"), F("Full update"));
        publishWidgets();
        publishState();

        sendAllFilesToMQTT();

#ifdef GATE_MODE
        publishTimes();
#endif
#ifdef EnableLogging
        choose_log_date_and_send();
#endif
    }

    else if (topicStr.indexOf("control") != -1) {
        String key = selectFromMarkerToMarker(topicStr, "/", 3);
        generateOrder(key, payloadStr);
        SerialPrint("i", F("=>MQTT"), "Msg from iotmanager app: " + key + " " + payloadStr);
    }

    //здесь мы получаем события с других устройств, которые потом проверяются в сценариях этого устройства
    else if (topicStr.indexOf("event") != -1) {
        //пока не работает сетевой обмен этот код будет закомментирован

        // if (!jsonReadBool(settingsFlashJson, "mqttin")) {
        //     return;
        // }
        // if (topicStr.indexOf(chipId) == -1) {
        //     String devId = selectFromMarkerToMarker(topicStr, "/", 2);
        //     String id = selectFromMarkerToMarker(topicStr, "/", 3);
        //     IoTItem* itemExist = findIoTItem(id);
        //     if (itemExist) {
        //         String valAsStr;
        //         if (jsonRead(payloadStr, F("val"), valAsStr, false)) {
        //             itemExist->setValue(valAsStr);
        //             unsigned long interval;
        //             jsonRead(payloadStr, F("int"), interval);
        //             itemExist->setInterval(interval);
        //         }
        //     } else {
        //         //добавим событие в базу
        //         // IoTItems.push_back((IoTItem*)new externalVariable(payloadStr));
        //     }
        //     //запустим проверку его в сценариях
        //     generateEvent(id, payloadStr);
        //     SerialPrint("i", F("=>MQTT"), "Received event from other device: '" + devId + "' " + id + " " + payloadStr);
        // }
    }

    //здесь мы получаем прямые команды которые сразу выполнятся на этом устройстве
    //необходимо для тех кто хочет управлять своим устройством из mqtt
    else if (topicStr.indexOf("order") != -1) {
        if (!jsonReadBool(settingsFlashJson, "mqttin")) {
            return;
        }
        String devId = selectFromMarkerToMarker(topicStr, "/", 2);
        String id = selectFromMarkerToMarker(topicStr, "/", 3);
        generateOrder(id, payloadStr);
        SerialPrint("i", F("=>MQTT"), "Received direct order " + id + " " + payloadStr);
    }

    // else if (topicStr.indexOf("info") != -1) {
    //    if (topicStr.indexOf("scen") != -1) {
    //        writeFile(String(DEVICE_SCENARIO_FILE), payloadStr);
    //        loadScenario();
    //        SerialPrint("i", F("=>MQTT"), F("Scenario received"));
    //    }
    //}
}

boolean publish(const String& topic, const String& data) {
    if (mqtt.beginPublish(topic.c_str(), data.length(), false)) {
        mqtt.print(data);
        return mqtt.endPublish();
    }
    return false;
}

boolean publishData(const String& topic, const String& data) {
    String path = mqttRootDevice + "/" + topic;
    if (!publish(path, data)) {
        SerialPrint("E", F("MQTT"), F("on publish data"));
        return false;
    }
    return true;
}

boolean publishChart(const String& topic, const String& data) {
    String path = mqttRootDevice + "/" + topic + "/status";
    if (!publish(path, data)) {
        SerialPrint("E", F("MQTT"), F("on publish chart"));
        return false;
    }
    return true;
}

boolean publishControl(String id, String topic, String state) {
    String path = mqttPrefix + "/" + id + "/" + topic + "/control";
    return mqtt.publish(path.c_str(), state.c_str(), false);
}

boolean publishChart_test(const String& topic, const String& data) {
    String path = mqttRootDevice + "/" + topic + "/status";
    return mqtt.publish(path.c_str(), data.c_str(), false);
}

boolean publishStatusMqtt(const String& topic, const String& data) {
    String path = mqttRootDevice + "/" + topic + "/status";
    String json = "{}";
    jsonWriteStr(json, "status", data);
    return mqtt.publish(path.c_str(), json.c_str(), false);
}

boolean publishAnyJsonKey(const String& topic, const String& key, const String& data) {
    String path = mqttRootDevice + "/" + topic + "/status";
    String json = "{}";
    jsonWriteStr(json, key, data);
    return mqtt.publish(path.c_str(), json.c_str(), false);
}

boolean publishEvent(const String& topic, const String& data) {
    String path = mqttRootDevice + "/" + topic + "/event";
    return mqtt.publish(path.c_str(), data.c_str(), false);
}

boolean publishInfo(const String& topic, const String& data) {
    String path = mqttRootDevice + "/" + topic + "/info";
    return mqtt.publish(path.c_str(), data.c_str(), false);
}

void publishWidgets() {
    auto file = seekFile("layout.json");
    if (!file) {
        SerialPrint("E", F("MQTT"), F("no file layout.json"));
        return;
    }
    size_t size = file.size();
    DynamicJsonDocument doc(size * 2);
    DeserializationError error = deserializeJson(doc, file);
    if (error) {
        SerialPrint("E", F("MQTT"), error.f_str());
        handleError("jse3", 1);
    }
    JsonArray arr = doc.as<JsonArray>();
    for (JsonVariant value : arr) {
        publishData("config", value.as<String>());
    }
    file.close();
}

void publishState() {
    String json = getParamsJson();
    json.replace("{", "");
    json.replace("}", "");
    json.replace("\"", "");
    json += ",";
    while (json.length() != 0) {
        String tmp = selectToMarker(json, ",");
        String topic = selectToMarker(tmp, ":");
        String state = deleteBeforeDelimiter(tmp, ":");
        if (topic != "" && state != "") {
            publishStatusMqtt(topic, state);
        }
        json = deleteBeforeDelimiter(json, ",");
    }
}

void handleMqttStatus(bool send) {
    String stateStr = getStateStr(mqtt.state());
    // Serial.println(stateStr);
    jsonWriteStr_(errorsHeapJson, F("mqtt"), stateStr);
    if (!send) standWebSocket.broadcastTXT(errorsHeapJson);
}

void handleMqttStatus(bool send, int state) {
    String stateStr = getStateStr(state);
    // Serial.println(stateStr);
    jsonWriteStr_(errorsHeapJson, F("mqtt"), stateStr);
    if (!send) standWebSocket.broadcastTXT(errorsHeapJson);
}

// log-20384820.txt
void sendAllFilesToMQTT() {
    String directory = "logs";
    SerialPrint("I", "Loging", "in directory '" + directory + "' files:");
    auto dir = FileFS.openDir(directory);
    String json_array;

    String reqUnixTimeStr = "25.08.2022";
    unsigned long reqUnixTime = strDateToUnix(reqUnixTimeStr);

    while (dir.next()) {
        String fname = dir.fileName();
        String id = selectToMarker(fname, "-");
        unsigned long fileUnixTime = deleteBeforeDelimiter(deleteToMarkerLast(fname, "."), "-").toInt() + START_DATETIME;

        SerialPrint("I", "Loging", "found file '" + String(fileUnixTime) + "'");

        if (isItemExist(id)) {
            //выбираем только те файлы которые входят в выбранные сутки
            if (fileUnixTime > reqUnixTime && fileUnixTime < reqUnixTime + 86400) {
                SerialPrint("I", "Loging", "file sent to MQTT: '" + fname + "'");
                createOneSingleJson(json_array, "/logs/" + fname);
            }
        } else {
            SerialPrint("i", "Loging", "file '" + fname + "' not used, deleted");
            removeFile(directory + "/" + fname);
        }
    }
    Serial.println("final: ");
    Serial.println(json_array);
}

void createOneSingleJson(String& json_array, String file) {
    File configFile = FileFS.open(file, "r");
    if (!configFile) {
        return;
    }
    configFile.seek(0, SeekSet);
    int i = 0;
    String buf = "{}";
    String unix_time;
    String value;
    unsigned int psn;
    unsigned int sz = configFile.size();
    do {
        i++;
        psn = configFile.position();
        String line = configFile.readStringUntil('\n');
        unix_time = selectToMarker(line, " ");
        jsonWriteInt(buf, "x", unix_time.toInt() + START_DATETIME);
        value = deleteBeforeDelimiter(line, " ");
        jsonWriteFloat(buf, "y1", value.toFloat());
        if (unix_time != "" || value != "") {
            json_array += buf + ",";
        }

    } while (psn < sz);

    Serial.println(file);
    Serial.println(json_array);

    configFile.close();

    // json_array = "{\"status\":[" + json_array + "]}";
    // json_array.replace("},]}", "}]}");
    // publishChart(topic, json_array);
}

const String getStateStr(int e) {
    switch (e) {
        case -4:  //Нет ответа от сервера
            return F("e1");
            break;
        case -3:  //Соединение было разорвано
            return F("e2");
            break;
        case -2:  //Ошибка соединения. Обычно возникает когда неверно указано название сервера MQTT
            return F("e3");
            break;
        case -1:  //Клиент был отключен
            return F("e4");
            break;
        case 0:  //подключено
            return F("e5");
            break;
        case 1:  //Ошибка версии
            return F("e6");
            break;
        case 2:  //Отклонен идентификатор
            return F("e7");
            break;
        case 3:  //Не могу установить соединение
            return F("e8");
            break;
        case 4:  //Неправильное имя пользователя/пароль
            return F("e9");
            break;
        case 5:  //Не авторизован для подключения
            return F("e10");
            break;
        case 6:  //Название сервера пустое
            return F("e11");
            break;
        case 7:  //Имя пользователя или пароль пустые
            return F("e12");
            break;
        case 8:  //Подключение в процессе
            return F("e13");
            break;
        default:
            return F("unk");
            break;
    }
}

void mqttUptimeCalc() {
    static unsigned int prevMillis;
    mqttUptime = mqttUptime + (millis() - prevMillis);
    prevMillis = millis();
}

void wifiUptimeCalc() {
    static unsigned int prevMillis;
    wifiUptime = wifiUptime + (millis() - prevMillis);
    prevMillis = millis();
}
