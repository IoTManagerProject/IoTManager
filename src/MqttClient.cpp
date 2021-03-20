#include "MqttClient.h"

#include "BufferExecute.h"
#include "Class/NotAsync.h"
#include "Global.h"
#include "Init.h"
#include "items/vLogging.h"
#include "items/vSensorNode.h"

void mqttInit() {
    myNotAsyncActions->add(
        do_MQTTPARAMSCHANGED, [&](void*) {
            mqttReconnect();
        },
        nullptr);

    mqtt.setCallback(mqttCallback);

    ts.add(
        WIFI_MQTT_CONNECTION_CHECK, MQTT_RECONNECT_INTERVAL,
        [&](void*) {
            if (WiFi.status() == WL_CONNECTED) {
                SerialPrint("I", "WIFI", "OK");
                if (mqtt.connected()) {
                    SerialPrint("I", "MQTT", "OK, broker No " + String(currentBroker));
                    setLedStatus(LED_OFF);
                } else {
                    SerialPrint("E", "MQTT", "connection lost");
                    mqttConnect();
                }
            } else {
                SerialPrint("E", "WIFI", "Lost WiFi connection");
                ts.remove(WIFI_MQTT_CONNECTION_CHECK);
                startAPMode();
            }
        },
        nullptr, true);

    myNotAsyncActions->add(
        do_sendScenMQTT, [&](void*) {
            String scen = readFile(String(DEVICE_SCENARIO_FILE), 2048);
            publishInfo("scen", scen);
        },
        nullptr);
}

void mqttDisconnect() {
    SerialPrint("I", "MQTT", "disconnected");
    mqtt.disconnect();
}

void mqttReconnect() {
    mqttDisconnect();
    mqttConnect();
}

void mqttLoop() {
    if (!isNetworkActive() || !mqtt.connected()) {
        return;
    }
    mqtt.loop();
}

void mqttSubscribe() {
    SerialPrint("I", "MQTT", "subscribed");
    mqtt.subscribe(mqttPrefix.c_str());
    mqtt.subscribe((mqttRootDevice + "/+/control").c_str());
    mqtt.subscribe((mqttRootDevice + "/update").c_str());

    if (jsonReadBool(configSetupJson, "MqttIn")) {
        mqtt.subscribe((mqttPrefix + "/+/+/event").c_str());
        mqtt.subscribe((mqttPrefix + "/+/+/order").c_str());
        mqtt.subscribe((mqttPrefix + "/+/+/info").c_str());
    }
}

void getMqttData1() {
    currentBroker = 1;
    mqttServer = jsonReadStr(configSetupJson, "mqttServer");
    mqttPort = jsonReadInt(configSetupJson, "mqttPort");
    mqttPrefix = jsonReadStr(configSetupJson, "mqttPrefix");
    mqttUser = jsonReadStr(configSetupJson, "mqttUser");
    mqttPass = jsonReadStr(configSetupJson, "mqttPass");
}

void getMqttData2() {
    currentBroker = 2;
    mqttServer = jsonReadStr(configSetupJson, "mqttServer2");
    mqttPort = jsonReadInt(configSetupJson, "mqttPort2");
    mqttPrefix = jsonReadStr(configSetupJson, "mqttPrefix2");
    mqttUser = jsonReadStr(configSetupJson, "mqttUser2");
    mqttPass = jsonReadStr(configSetupJson, "mqttPass2");
}

bool isSecondBrokerSet() {
    bool res = true;
    if (jsonReadStr(configSetupJson, "mqttServer2") == "") {
        res = false;
    }
    if (jsonReadStr(configSetupJson, "mqttPrefix2") == "") {
        res = false;
    }
    return res;
}

boolean mqttConnect() {

    if (changeBroker) {
        if (currentBroker == 1) {
            getMqttData2();
        } else if (currentBroker == 2) {
            getMqttData1();
        }
    } else {
        getMqttData1();
    }

    bool res = false;
    if (mqttServer == "") {
        SerialPrint("E", "MQTT", "mqttServer empty");
        return res;
    }
    SerialPrint("I", "MQTT", "connection started to broker No " + String(currentBroker));

    mqttRootDevice = mqttPrefix + "/" + chipId;

    SerialPrint("I", "MQTT", "broker " + mqttServer + ":" + String(mqttPort, DEC));
    SerialPrint("I", "MQTT", "topic " + mqttRootDevice);
    setLedStatus(LED_FAST);
    mqtt.setServer(mqttServer.c_str(), mqttPort);

    if (!mqtt.connected()) {
        if (mqtt.connect(chipId.c_str(), mqttUser.c_str(), mqttPass.c_str())) {
            SerialPrint("I", "MQTT", "connected");
            setLedStatus(LED_OFF);
            mqttSubscribe();
            res = true;
        } else {
            mqttConnectAttempts++;
            SerialPrint("E", "MQTT", "🡆 Attempt No: " + String(mqttConnectAttempts) + " could't connect, retry in " + String(MQTT_RECONNECT_INTERVAL / 1000) + "s");
            setLedStatus(LED_FAST);

            if (mqttConnectAttempts >= CHANGE_BROKER_AFTER) {
                mqttConnectAttempts = 0;
                if (isSecondBrokerSet()) {
                    changeBroker = true;
                    SerialPrint("E", "MQTT", "Broker fully missed (" + String(CHANGE_BROKER_AFTER) + " attempts passed), try connect to another one");
                } else {
                    SerialPrint("E", "MQTT", "Secound broker not seted");
                }
            }
        }
    }
    return res;
}

void mqttCallback(char* topic, uint8_t* payload, size_t length) {
    String topicStr = String(topic);
    //SerialPrint("I", "=>MQTT", topicStr);
    String payloadStr;
    payloadStr.reserve(length + 1);
    for (size_t i = 0; i < length; i++) {
        payloadStr += (char)payload[i];
    }

    //SerialPrint("I", "=>MQTT", payloadStr);

    if (payloadStr.startsWith("HELLO")) {
        SerialPrint("I", "MQTT", "Full update");
        publishWidgets();
        publishState();
#ifdef GATE_MODE
        publishTimes();
#endif
#ifdef EnableLogging
        choose_log_date_and_send();
#endif
    }

    else if (topicStr.indexOf("control") != -1) {
        String key = selectFromMarkerToMarker(topicStr, "/", 3);

        String order;
        order += key;
        order += " ";
        order += payloadStr;
        order += ",";

        loopCmdAdd(order);

        SerialPrint("I", "=>MQTT", "Msg from iotmanager app: " + key + " " + payloadStr);
    }

    else if (topicStr.indexOf("event") != -1) {
        if (!jsonReadBool(configSetupJson, "MqttIn")) {
            return;
        }
        if (topicStr.indexOf(chipId) == -1) {
            String devId = selectFromMarkerToMarker(topicStr, "/", 2);
            String key = selectFromMarkerToMarker(topicStr, "/", 3);
            SerialPrint("I", "=>MQTT", "Received event from other device: '" + devId + "' " + key + " " + payloadStr);
            String event = key + " " + payloadStr + ",";
            eventBuf += event;
        }
    }

    else if (topicStr.indexOf("order") != -1) {
        if (!jsonReadBool(configSetupJson, "MqttIn")) {
            return;
        }
        String devId = selectFromMarkerToMarker(topicStr, "/", 2);
        String key = selectFromMarkerToMarker(topicStr, "/", 3);
        SerialPrint("I", "=>MQTT", "Received direct order " + key + " " + payloadStr);
        String order = key + " " + payloadStr + ",";
        orderBuf += order;
    }

    else if (topicStr.indexOf("info") != -1) {
        if (topicStr.indexOf("scen") != -1) {
            writeFile(String(DEVICE_SCENARIO_FILE), payloadStr);
            loadScenario();
            SerialPrint("I", "=>MQTT", "Scenario received");
        }
    }
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
        SerialPrint("[E]", "MQTT", "on publish data");
        return false;
    }
    return true;
}

boolean publishChart(const String& topic, const String& data) {
    String path = mqttRootDevice + "/" + topic + "/status";
    if (!publish(path, data)) {
        SerialPrint("[E]", "MQTT", "on publish chart");
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

boolean publishStatus(const String& topic, const String& data) {
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
            //jsonWriteStr(line, "id", String(counter));
            //jsonWriteStr(line, "pageId", String(counter));
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
        SerialPrint("[E]", "MQTT", "no file layout.txt");
        return;
    }
    while (file.available()) {
        String payload = file.readStringUntil('\n');
        SerialPrint("I", "MQTT", "widgets: " + payload);
        publishData("config", payload);
    }
    file.close();
}
#endif

void publishState() {
    // берет строку json и ключи превращает в топики а значения колючей в них посылает
    String str;
    if (configLiveJson != "{}") {
        str += configLiveJson;
    }
    if (configStoreJson != "{}") {
        str += "," + configStoreJson;
    }
    str.replace("{", "");
    str.replace("}", "");
    str.replace("\"", "");
    str += ",";

    while (str.length() != 0) {
        String tmp = selectToMarker(str, ",");

        String topic = selectToMarker(tmp, ":");
        String state = deleteBeforeDelimiter(tmp, ":");

        if (topic != "" && state != "") {
            if (topic != "timenow") {
                publishStatus(topic, state);
            }
        }
        str = deleteBeforeDelimiter(str, ",");
    }
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
