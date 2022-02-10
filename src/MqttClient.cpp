#include "MqttClient.h"

void mqttInit() {
    mqtt.setCallback(mqttCallback);
    ts.add(
        WIFI_MQTT_CONNECTION_CHECK, MQTT_RECONNECT_INTERVAL,
        [&](void*) {
            if (WiFi.status() == WL_CONNECTED) {
                SerialPrint("I", F("WIFI"), F("OK"));
                if (mqtt.connected()) {
                    SerialPrint("I", F("MQTT"), "OK");
                    jsonWriteInt_(errorsHeapJson, F("mqtt"), mqtt.state());
                    // setLedStatus(LED_OFF);
                } else {
                    SerialPrint("E", F("MQTT"), F("✖ Connection lost"));
                    jsonWriteInt_(errorsHeapJson, F("mqtt"), mqtt.state());
                    mqttConnect();
                }
            } else {
                SerialPrint("E", F("WIFI"), F("✖ Lost WiFi connection"));
                ts.remove(WIFI_MQTT_CONNECTION_CHECK);
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
        jsonWriteInt_(errorsHeapJson, F("mqtt"), 6);
        return res;
    }
    SerialPrint("I", "MQTT", "connection started");

    SerialPrint("I", "MQTT", "broker " + mqttServer + ":" + String(mqttPort, DEC));
    SerialPrint("I", "MQTT", "topic " + mqttRootDevice);

    // setLedStatus(LED_FAST);

    mqtt.setServer(mqttServer.c_str(), mqttPort);

    if (!mqtt.connected()) {
        bool connected = false;
        if (mqttUser != "" && mqttPass != "") {
            connected = mqtt.connect(chipId.c_str(), mqttUser.c_str(), mqttPass.c_str());
            SerialPrint("I", F("MQTT"), F("Go to connection with login and password"));
        } else if (mqttUser == "" && mqttPass == "") {
            connected = mqtt.connect(chipId.c_str());
            SerialPrint("I", F("MQTT"), F("Go to connection without login and password"));
        } else {
            SerialPrint("E", F("MQTT"), F("✖ Login or password missed"));
            jsonWriteInt_(errorsHeapJson, F("mqtt"), 7);
            return res;
        }

        if (connected) {
            SerialPrint("I", F("MQTT"), F("✔ connected"));
            jsonWriteInt_(errorsHeapJson, F("mqtt"), mqtt.state());
            //  setLedStatus(LED_OFF);
            mqttSubscribe();
            res = true;
        } else {
            SerialPrint("E", F("MQTT"), "🡆 Could't connect, retry in " + String(MQTT_RECONNECT_INTERVAL / 1000) + "s");
            jsonWriteInt_(errorsHeapJson, F("mqtt"), mqtt.state());
            // setLedStatus(LED_FAST);
        }
    }
    return res;
}

void mqttDisconnect() {
    SerialPrint("I", F("MQTT"), F("disconnected"));
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
    SerialPrint("I", F("MQTT"), F("subscribed"));
    SerialPrint("I", F("MQTT"), mqttRootDevice);
    mqtt.subscribe(mqttPrefix.c_str());
    mqtt.subscribe((mqttRootDevice + "/+/control").c_str());
    mqtt.subscribe((mqttRootDevice + "/update").c_str());

    if (jsonReadBool(settingsFlashJson, "MqttIn")) {
        mqtt.subscribe((mqttPrefix + "/+/+/event").c_str());
        mqtt.subscribe((mqttPrefix + "/+/+/order").c_str());
        mqtt.subscribe((mqttPrefix + "/+/+/info").c_str());
    }
}

void mqttCallback(char* topic, uint8_t* payload, size_t length) {
    String topicStr = String(topic);
    // SerialPrint("I", "=>MQTT", topicStr);
    String payloadStr;
    payloadStr.reserve(length + 1);
    for (size_t i = 0; i < length; i++) {
        payloadStr += (char)payload[i];
    }

    // SerialPrint("I", "=>MQTT", payloadStr);

    if (payloadStr.startsWith("HELLO")) {
        SerialPrint("I", F("MQTT"), F("Full update"));
        publishWidgets();
        publishState();
#ifdef GATE_MODE
        publishTimes();
#endif
#ifdef EnableLogging
        choose_log_date_and_send();
#endif
    }

    // else if (topicStr.indexOf("control") != -1) {
    //     String key = selectFromMarkerToMarker(topicStr, "/", 3);
    //
    //    String order;
    //    order += key;
    //    order += " ";
    //    order += payloadStr;
    //    order += ",";
    //
    //    loopCmdAdd(order);
    //
    //    SerialPrint("I", F("=>MQTT"), "Msg from iotmanager app: " + key + " " + payloadStr);
    //}
    //
    // else if (topicStr.indexOf("event") != -1) {
    //    if (!jsonReadBool(settingsFlashJson, "MqttIn")) {
    //        return;
    //    }
    //    if (topicStr.indexOf(chipId) == -1) {
    //        String devId = selectFromMarkerToMarker(topicStr, "/", 2);
    //        String key = selectFromMarkerToMarker(topicStr, "/", 3);
    //        SerialPrint("I", F("=>MQTT"), "Received event from other device: '" + devId + "' " + key + " " + payloadStr);
    //        String event = key + " " + payloadStr + ",";
    //        eventBuf += event;
    //    }
    //}
    //
    // else if (topicStr.indexOf("order") != -1) {
    //    if (!jsonReadBool(settingsFlashJson, "MqttIn")) {
    //        return;
    //    }
    //    String devId = selectFromMarkerToMarker(topicStr, "/", 2);
    //    String key = selectFromMarkerToMarker(topicStr, "/", 3);
    //    SerialPrint("I", F("=>MQTT"), "Received direct order " + key + " " + payloadStr);
    //    String order = key + " " + payloadStr + ",";
    //    loopCmdAdd(order);
    //    SerialPrint("I", "Order add", order);
    //}
    //
    // else if (topicStr.indexOf("info") != -1) {
    //    if (topicStr.indexOf("scen") != -1) {
    //        writeFile(String(DEVICE_SCENARIO_FILE), payloadStr);
    //        loadScenario();
    //        SerialPrint("I", F("=>MQTT"), F("Scenario received"));
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

#ifdef LAYOUT_IN_RAM
void publishWidgets() {
    if (all_widgets != "") {
        int counter = 0;
        String line;
        int psn_1 = 0;
        int psn_2;
        do {
            psn_2 = all_widgets.indexOf("\r\n", psn_1);  //\r\n
            line = all_widgets.substring(psn_1, psn_2);
            line.replace("\n", "");
            line.replace("\r\n", "");
            // jsonWriteStr(line, "id", String(counter));
            // jsonWriteStr(line, "pageId", String(counter));
            counter++;
            sendMQTT("config", line);
            Serial.println("[V] " + line);
            psn_1 = psn_2 + 1;
        } while (psn_2 + 2 < all_widgets.length());
        getMemoryLoad("I after send all widgets");
    }
}
#endif

#ifndef LAYOUT_IN_RAM
void publishWidgets() {
    auto file = seekFile("layout.txt");
    if (!file) {
        SerialPrint("E", F("MQTT"), F("no file layout.txt"));
        return;
    }
    while (file.available()) {
        String payload = file.readStringUntil('\n');
        SerialPrint("I", F("MQTT"), "widgets: " + payload);
        publishData("config", payload);
    }
    file.close();
}
#endif

void publishState() {
    //// берет строку json и ключи превращает в топики а значения колючей в них посылает
    // String str;
    // if (paramsHeapJson != "{}") {
    //     str += paramsHeapJson;
    // }
    // if (paramsFlashJson != "{}") {
    //     str += "," + paramsFlashJson;
    // }
    // str.replace("{", "");
    // str.replace("}", "");
    // str.replace("\"", "");
    // str += ",";
    //
    // while (str.length() != 0) {
    //    String tmp = selectToMarker(str, ",");
    //
    //    String topic = selectToMarker(tmp, ":");
    //    String state = deleteBeforeDelimiter(tmp, ":");
    //
    //    if (topic != "" && state != "") {
    //        if (topic != "timenow") {
    //            publishStatusMqtt(topic, state);
    //        }
    //    }
    //    str = deleteBeforeDelimiter(str, ",");
    //}
}

const String getStateStr() {
    switch (mqtt.state()) {
        case -4:
            return F("no respond");
            break;
        case -3:
            return F("connection was broken");
            break;
        case -2:
            return F("connection failed");
            break;
        case -1:
            return F("client disconnected");
            break;
        case 0:
            return F("client connected");
            break;
        case 1:
            return F("doesn't support the requested version");
            break;
        case 2:
            return F("rejected the client identifier");
            break;
        case 3:
            return F("unable to accept the connection");
            break;
        case 4:
            return F("wrong username/password");
            break;
        case 5:
            return F("not authorized to connect");
            break;
        default:
            return F("unspecified");
            break;
    }
}

// 6 сервер не задан
// 7 Логин или пароль отсутствует
