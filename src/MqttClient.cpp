#include "MqttClient.h"
#include "Global.h"
#include <LittleFS.h>

static const char* MODULE = "Mqtt";

namespace MqttClient {

// Errors
int wifi_lost_error = 0;
int mqtt_lost_error = 0;
bool connected = false;

// Session params
String mqttPrefix;
String mqttRootDevice;
int port;
String user;
String pass;
String addr;

void init() {
    mqtt.setCallback(handleSubscribedUpdates);

    ts.add(
        WIFI_MQTT_CONNECTION_CHECK, MQTT_RECONNECT_INTERVAL,
        [&](void*) {
            if (isNetworkActive()) {
                if (mqtt.connected()) {
                    if (!connected) {
                        pm.info(String("OK"));
                        setLedStatus(LED_OFF);
                        connected = true;
                    }
                } else {
                    connect();
                    if (!just_load) mqtt_lost_error++;
                }
            } else {
                if (connected) {
                    pm.error(String("connection lost"));
                    connected = false;
                }
                ts.remove(WIFI_MQTT_CONNECTION_CHECK);
                wifi_lost_error++;
                startAPMode();
            }
        },
        nullptr, true);
}

void disconnect() {
    pm.info(String("disconnect"));
    mqtt.disconnect();
}

void reconnect() {
    disconnect();
    connect();
}

void loop() {
    if (!isNetworkActive() || !mqtt.connected()) {
        return;
    }
    mqtt.loop();
}

void subscribe() {
    pm.info(String("subscribe"));
    mqtt.subscribe(mqttPrefix.c_str());
    mqtt.subscribe((mqttRootDevice + "/+/control").c_str());
    mqtt.subscribe((mqttRootDevice + "/order").c_str());
    mqtt.subscribe((mqttRootDevice + "/update").c_str());
    mqtt.subscribe((mqttRootDevice + "/devc").c_str());
    mqtt.subscribe((mqttRootDevice + "/devs").c_str());
}

bool readParams() {
    addr = jsonReadStr(configSetupJson, "mqttServer");
    if (!addr) {
        pm.error(String("no broker address"));
        return false;
    }
    port = jsonReadInt(configSetupJson, "mqttPort");
    user = jsonReadStr(configSetupJson, "mqttUser");
    pass = jsonReadStr(configSetupJson, "mqttPass");
    mqttPrefix = jsonReadStr(configSetupJson, "mqttPrefix");
    mqttRootDevice = mqttPrefix + "/" + chipId;
    return true;
}

boolean connect() {
    pm.info(String("connect"));

    if (!readParams()) {
        return false;
    }

    pm.info("broker " + addr + ":" + String(port, DEC));
    pm.info("topic " + mqttRootDevice);

    setLedStatus(LED_FAST);

    mqtt.setServer(addr.c_str(), port);
    bool res = false;
    if (!mqtt.connected()) {
        if (mqtt.connect(chipId.c_str(), user.c_str(), pass.c_str())) {
            pm.info(String("connected"));
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
    pm.info("got updates: " + topicStr);
    String payloadStr = "";
    payloadStr.reserve(length + 1);
    for (size_t i = 0; i < length; i++) {
        payloadStr += (char)payload[i];
    }

    if (payloadStr.equalsIgnoreCase("hello")) {
        pm.info(String("hello"));
        publishWidgets();
        publishState();
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
        addCommandLoop(topic + " " + number + " " + payloadStr);
    } else if (topicStr.indexOf("order")) {
        payloadStr.replace("_", " ");
        addCommandLoop(payloadStr);
    } else if (topicStr.indexOf("update")) {
        if (payloadStr == "1") {
            updateFlag = true;
        }
    } else if (topicStr.indexOf("devc")) {
        writeFile(String(DEVICE_CONFIG_FILE), payloadStr);
        Device_init();
    } else if (topicStr.indexOf("devs")) {
        writeFile(String(DEVICE_SCENARIO_FILE), payloadStr);
        loadScenario();
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
        pm.error(String("on publish"));
        return false;
    }
    return true;
}

boolean publishChart(const String& topic, const String& data) {
    String path = mqttRootDevice + "/" + topic + "/status";
    if (!publish(path, data)) {
        pm.error(String("on publish"));
        return false;
    }
    return true;
}

boolean publishControl(String id, String topic, String state) {
    String path = mqttPrefix + "/" + id + "/" + topic + "/control";
    return mqtt.publish(path.c_str(), state.c_str(), false);
}

boolean publishStatus(const String& topic, const String& data) {
    String path = mqttRootDevice + "/" + topic + "/status";
    String json = "{}";
    jsonWriteStr(json, "status", data);
    return mqtt.publish(path.c_str(), json.c_str());
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
        getMemoryLoad("[I] after send all widgets");
    }
}
#endif

#ifndef LAYOUT_IN_RAM
void publishWidgets() {
    auto file = seekFile("layout.txt");
    if (!file) {
        pm.error(String("on seek layout.txt"));
        return;
    }
    while (file.available()) {
        String payload = file.readStringUntil('\n');
        pm.info("widgets: " + payload);
        publishData("config", payload);
    }
    file.close();
}
#endif

bool isExludedTopic(const String& topic) {
    return (topic == "time") || (topic == "name") || (topic == "lang") || (topic == "ip") || (topic.indexOf("_in") < 0);
}

void publishState() {
    // берет строку json и ключи превращает в топики а значения колючей в них посылает
    // {"name":"MODULES","lang":"","ip":"192.168.43.60","DS":"34.00","rel1":"1","rel2":"1"}
    // "name":"MODULES","lang":"","ip":"192.168.43.60","DS":"34.00","rel1":"1","rel2":"1"
    // "name":"MODULES","lang":"","ip":"192.168.43.60","DS":"34.00","rel1":"1","rel2":"1",
    String str = configLiveJson;
    str.replace("{", "");
    str.replace("}", "");
    str += ",";

    while (str.length()) {
        String tmp = selectToMarker(str, ",");

        String topic = selectToMarker(tmp, ":");
        topic.replace("\"", "");

        String state = selectToMarkerLast(tmp, ":");
        state.replace("\"", "");

        if (!isExludedTopic(topic)) {
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
}  // namespace MqttClient