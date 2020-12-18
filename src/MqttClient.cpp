#include "MqttClient.h"

#include <LittleFS.h>
#include "items/vLogging.h"
#include "Class/NotAsync.h"
#include "Global.h"
#include "Init.h"

enum MqttBroker {MQTT_PRIMARY, MQTT_RESERVE};
MqttBroker activeBroker = MQTT_PRIMARY;

String mqttPrefix;
String mqttRootDevice;
String mqttPass;
String mqttServer;
String mqttUser;
uint16_t mqttPort{0};
uint16_t reconnectionCounter{0};
bool primaryExist = false;

const String getParamName(const char* param, MqttBroker broker) {
    return String("mqtt") + param + (broker == MQTT_RESERVE? "2": "");
}

bool checkBrokerParams(MqttBroker broker) {
    return !jsonReadStr(configSetupJson, getParamName("Server", broker)).isEmpty();   
}

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
                    SerialPrint("I", "MQTT", "OK");
                    setLedStatus(LED_OFF);
                }
                else {
                    SerialPrint("E", "MQTT", "lost connection");
                    if (reconnectionCounter++ > 5) {
                        if (activeBroker == MQTT_PRIMARY) {
                            if (checkBrokerParams(MQTT_RESERVE)) {
                                activeBroker = MQTT_RESERVE;                             
                            }
                        } else {
                            activeBroker = MQTT_PRIMARY;
                        }
                        reconnectionCounter = 0;
                    }
                    mqttConnect();
                }
            }
            else {
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
    SerialPrint("I", "MQTT", "disconnect");
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
    SerialPrint("I", "MQTT", "subscribe");
    mqtt.subscribe(mqttPrefix.c_str());
    mqtt.subscribe((mqttRootDevice + "/+/control").c_str());
    mqtt.subscribe((mqttRootDevice + "/update").c_str());

    if (jsonReadBool(configSetupJson, "MqttIn")) {
        mqtt.subscribe((mqttPrefix + "/+/+/event").c_str());
        mqtt.subscribe((mqttPrefix + "/+/+/info").c_str());
    }
}

bool readBrokerParams(MqttBroker broker) {
    if(!checkBrokerParams(broker)) {
        return false;
    }
    mqttServer = jsonReadStr(configSetupJson, getParamName("Server", broker));
    mqttPort = jsonReadInt(configSetupJson, getParamName("Port", broker));
    mqttUser = jsonReadStr(configSetupJson, getParamName("User", broker));
    mqttPass = jsonReadStr(configSetupJson, getParamName("Pass", broker));

    return true;
}

boolean mqttConnect() {
    SerialPrint("I", "MQTT", String("use ") + (activeBroker == MQTT_PRIMARY? "primary": "reserve"));    
    if (!checkBrokerParams(activeBroker)) {
        SerialPrint("E", "MQTT", "empty broker address");
        return false;
    }
    readBrokerParams(activeBroker);
    SerialPrint("I", "MQTT", "start connection");
    mqttPrefix = jsonReadStr(configSetupJson, "mqttPrefix");
    mqttRootDevice = mqttPrefix + "/" + chipId;
    SerialPrint("I", "MQTT", "broker " + mqttServer + ":" + String(mqttPort, DEC));
    SerialPrint("I", "MQTT", "topic " + mqttRootDevice);
    setLedStatus(LED_FAST);
    mqtt.setServer(mqttServer.c_str(), mqttPort);
    bool res = false;
    if (!mqtt.connected()) {
        if (mqtt.connect(chipId.c_str(), mqttUser.c_str(), mqttPass.c_str())) {
            SerialPrint("I", "MQTT", "connected");
            setLedStatus(LED_OFF);
            mqttSubscribe();
            res = true;
        }
        else {
            SerialPrint("E", "MQTT", "could't connect, retry in " + String(MQTT_RECONNECT_INTERVAL / 1000) + "s");
            setLedStatus(LED_FAST);
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

        choose_log_date_and_send();
    }

    else if (topicStr.indexOf("control") != -1) {

        String key = selectFromMarkerToMarker(topicStr, "/", 3);

        orderBuf += key;
        orderBuf += " ";
        orderBuf += payloadStr;
        orderBuf += ",";

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

    else if (topicStr.indexOf("info") != -1) {
        if (topicStr.indexOf("scen") != -1) {
            writeFile(String(DEVICE_SCENARIO_FILE), payloadStr);
            loadScenario();
            SerialPrint("I", "=>MQTT", "Scenario received");
        }
    }

    //else if (topicStr.indexOf("update")) {
    //    if (payloadStr == "1") {
    //        myNotAsyncActions->make(do_UPGRADE);
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

boolean publishEvent(const String& topic, const String& data) {
    String path = mqttRootDevice + "/" + topic + "/event";
    return mqtt.publish(path.c_str(), data.c_str(), true);
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
    String str = configLiveJson + "," + configStoreJson;
    str.replace("{", "");
    str.replace("}", "");
    str += ",";

    while (str.length()) {
        String tmp = selectToMarker(str, ",");

        String topic = selectToMarker(tmp, "\":");
        topic.replace("\"", "");

        String state = selectToMarkerLast(tmp, "\":");
        state.replace("\"", "");

        if (topic != "timenow") {
            publishStatus(topic, state);
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
