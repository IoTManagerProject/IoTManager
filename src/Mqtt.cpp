#include "Global.h"

//
#include <LittleFS.h>

static const char* MODULE = "Mqtt";

// Errors
int wifi_lost_error = 0;
int mqtt_lost_error = 0;

// Session params
String mqttPrefix;

void handleSubscribedUpdates(char* topic, uint8_t* payload, size_t length);
const String getMqttStateStr();
void sendAllData();
void sendAllWigets();
boolean sendSTATUS(String topic, String state);
void outcoming_date();

void initMQTT() {
    mqtt.setCallback(handleSubscribedUpdates);

    ts.add(
        WIFI_MQTT_CONNECTION_CHECK, MQTT_RECONNECT_INTERVAL,
        [&](void*) {
            if (isNetworkActive()) {
                if (mqtt.connected()) {
                    pm.info("OK");
                    setLedStatus(LED_OFF);
                } else {
                    connectMQTT();
                    if (!just_load) mqtt_lost_error++;
                }
            } else {
                pm.error("WiFi connection lost");
                ts.remove(WIFI_MQTT_CONNECTION_CHECK);
                wifi_lost_error++;
                startAPMode();
            }
        },
        nullptr, true);
}

void do_mqtt_connection() {
    if (mqttParamsChanged) {
        mqtt.disconnect();
        connectMQTT();
        mqttParamsChanged = false;
    }
}

void loopMQTT() {
    if (!isNetworkActive() || !mqtt.connected()) {
        return;
    }
    mqtt.loop();
}

void subscribe() {
    pm.info("Subscribe");
    // Для приема получения HELLOW и подтверждения связи
    // Подписываемся на топики control
    // Подписываемся на топики order
    mqtt.subscribe(jsonReadStr(configSetupJson, "mqttPrefix").c_str());
    mqtt.subscribe((jsonReadStr(configSetupJson, "mqttPrefix") + "/" + chipId + "/+/control").c_str());
    mqtt.subscribe((jsonReadStr(configSetupJson, "mqttPrefix") + "/" + chipId + "/order").c_str());
    mqtt.subscribe((jsonReadStr(configSetupJson, "mqttPrefix") + "/" + chipId + "/update").c_str());
    mqtt.subscribe((jsonReadStr(configSetupJson, "mqttPrefix") + "/" + chipId + "/devc").c_str());
    mqtt.subscribe((jsonReadStr(configSetupJson, "mqttPrefix") + "/" + chipId + "/devs").c_str());
}

boolean connectMQTT() {
    if (!isNetworkActive()) {
        return false;
    }

    String addr = jsonReadStr(configSetupJson, "mqttServer");
    int port = jsonReadInt(configSetupJson, "mqttPort");
    String user = jsonReadStr(configSetupJson, "mqttUser");
    String pass = jsonReadStr(configSetupJson, "mqttPass");
    mqttPrefix = jsonReadStr(configSetupJson, "mqttPrefix");

    if (!addr) {
        pm.error("no broker address");
        return false;
    }
    pm.info("broker " + addr + ":" + String(port, DEC));
    setLedStatus(LED_FAST);
    mqtt.setServer(addr.c_str(), port);
    bool res = false;
    if (!mqtt.connected()) {
        if (mqtt.connect(chipId.c_str(), user.c_str(), pass.c_str())) {
            pm.info("connected");
            setLedStatus(LED_OFF);
            subscribe();
            res = true;
        } else {
            pm.error("could't connect, retry in " + String(MQTT_RECONNECT_INTERVAL / 1000) + "s");
            setLedStatus(LED_FAST);
        }
    }
    return res;
}

void handleSubscribedUpdates(char* topic, uint8_t* payload, size_t length) {
    String topicStr = String(topic);
    pm.info(topicStr);

    String payloadStr;
    payloadStr.reserve(length + 1);
    for (size_t i = 0; i < length; i++) {
        payloadStr += (char)payload[i];
    }
    pm.info(payloadStr);

    if (payloadStr == "HELLO") {
        //данные которые отправляем при подключении или отбновлении страницы
        pm.info("Send web page updates");
        sendAllWigets();
        sendAllData();
#ifdef LOGGING_ENABLED
        choose_log_date_and_send();
#endif
    } else if (topicStr.indexOf("control")) {
        // название топика -  команда,
        // значение - параметр
        //IoTmanager/800324-1458415/button99/control 1
        String topic = selectFromMarkerToMarker(topicStr, "/", 3);
        topic = add_set(topic);
        String number = selectToMarkerLast(topic, "Set");
        topic.replace(number, "");

        order_loop += topic;
        order_loop += " ";
        order_loop += number;
        order_loop += " ";
        order_loop += payloadStr;
        order_loop += ",";
    } else if (topicStr.indexOf("order")) {
        payloadStr.replace("_", " ");
        order_loop += payloadStr;
        order_loop += ",";
    } else if (topicStr.indexOf("update")) {
        if (payloadStr == "1") {
            upgrade = true;
        }
    } else if (topicStr.indexOf("devc")) {
        writeFile("firmware.c.txt", payloadStr);
        Device_init();
    } else if (topicStr.indexOf("devs")) {
        writeFile("firmware.s.txt", payloadStr);
        Scenario_init();
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
    String path = mqttPrefix + "/" + chipId + "/" + topic;
    if (!publish(path, data)) {
        pm.error("on publish data");
        return false;
    }
    return true;
}

boolean publishChart(const String& topic, const String& data) {
    String path = mqttPrefix + "/" + chipId + "/" + topic + "/status";
    if (!publish(path, data)) {
        pm.error("on publish chart");
        return false;
    }
    return true;
}

boolean publishControl(String id, String topic, String state) {
    String path = mqttPrefix + "/" + id + "/" + topic + "/control";
    return mqtt.publish(path.c_str(), state.c_str(), false);
}

boolean sendCHART_test(String topic, String data) {
    topic = mqttPrefix + "/" + chipId + "/" + topic + "/" + "status";
    return mqtt.publish(topic.c_str(), data.c_str(), false);
}

boolean sendSTATUS(String topic, String state) {
    topic = mqttPrefix + "/" + chipId + "/" + topic + "/" + "status";
    String json_ = "{}";
    jsonWriteStr(json_, "status", state);
    return mqtt.publish(topic.c_str(), json_.c_str(), false);
}

//=====================================================ОТПРАВЛЯЕМ ВИДЖЕТЫ========================================================
#ifdef LAYOUT_IN_RAM
void sendAllWigets() {
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
        getMemoryLoad("[I] after send all widgets");
    }
}
#endif

#ifndef LAYOUT_IN_RAM
void sendAllWigets() {
    auto file = seekFile("/layout.txt");
    if (!file) {
        return;
    }
    while (file.position() != file.size()) {
        String payload = file.readStringUntil('\n');
        pm.info("publish: " + payload);
        publishData("config", payload);
    }
    file.close();
}
#endif

//=====================================================ОТПРАВЛЯЕМ ДАННЫЕ В ВИДЖЕТЫ ПРИ ОБНОВЛЕНИИ СТРАНИЦЫ========================================================
void sendAllData() {  //берет строку json и ключи превращает в топики а значения колючей в них посылает

    String current_config = configLiveJson;  //{"name":"MODULES","lang":"","ip":"192.168.43.60","DS":"34.00","rel1":"1","rel2":"1"}
    printMemoryStatus("[I] after send all date");
    current_config.replace("{", "");
    current_config.replace("}", "");  //"name":"MODULES","lang":"","ip":"192.168.43.60","DS":"34.00","rel1":"1","rel2":"1"
    current_config += ",";            //"name":"MODULES","lang":"","ip":"192.168.43.60","DS":"34.00","rel1":"1","rel2":"1",

    while (current_config.length() != 0) {
        String tmp = selectToMarker(current_config, ",");
        String topic = selectToMarker(tmp, ":");
        topic.replace("\"", "");
        String state = selectToMarkerLast(tmp, ":");
        state.replace("\"", "");
        if (topic != "name" && topic != "lang" && topic != "ip" && topic.indexOf("_in") < 0) {
            sendSTATUS(topic, state);
            //Serial.println("-->" + topic + " " + state);
        }
        current_config = deleteBeforeDelimiter(current_config, ",");
    }
}

const String getMqttStateStr() {
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
            return F("client disconnected ");
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
