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
struct Params {
    String uuid;
    String prefix;
    String deviceRoot;
    String addr;
    int port;
    String user;
    String pass;
} params;

void init() {
    mqtt.setCallback(handleSubscribedUpdates);

    ts.add(
        WIFI_MQTT_CONNECTION_CHECK, MQTT_RECONNECT_INTERVAL,
        [&](void*) {
            if (isNetworkActive()) {
                if (mqtt.connected()) {
                    if (!connected) {
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
    if (isNetworkActive() && mqtt.connected()) {
        mqtt.loop();
    }
}

void subscribe(struct Params& p) {
    pm.info("subscribe");
    mqtt.subscribe(p.prefix.c_str());
    mqtt.subscribe((p.deviceRoot + "/+/control").c_str());
    mqtt.subscribe((p.deviceRoot + "/order").c_str());
    mqtt.subscribe((p.deviceRoot + "/update").c_str());
    mqtt.subscribe((p.deviceRoot + "/devc").c_str());
    mqtt.subscribe((p.deviceRoot + "/devs").c_str());
}

bool readParams(struct Params& p) {
    p.uuid = getChipId();
    p.addr = jsonReadStr(configSetupJson, "mqttServer");
    if (!p.addr.length()) {
        pm.error("no address");
        return false;
    }
    p.port = jsonReadInt(configSetupJson, "mqttPort");
    p.user = jsonReadStr(configSetupJson, "mqttUser");
    p.pass = jsonReadStr(configSetupJson, "mqttPass");

    p.prefix = jsonReadStr(configSetupJson, "mqttPrefix");
    p.deviceRoot = p.prefix + "/" + p.uuid;

    pm.info("broker " + params.addr + ":" + String(params.port, DEC));
    pm.info("root " + params.deviceRoot);

    return true;
}

boolean connect() {
    pm.info("connect");

    if (!readParams(params)) {
        return false;
    }
    setLedStatus(LED_FAST);

    bool res = false;
    mqtt.setServer(params.addr.c_str(), params.port);
    if (!mqtt.connected()) {
        if (mqtt.connect(params.uuid.c_str(), params.user.c_str(), params.pass.c_str())) {
            pm.info(String("connected"));
            setLedStatus(LED_OFF);
            subscribe(params);
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
    pm.error(String("on publish"));
    return false;
}

boolean publishData(const String& topic, const String& data) {
    String path = params.deviceRoot + "/" + topic;
    return publish(path, data);
}

boolean publishChart(const String& topic, const String& data) {
    String path = params.deviceRoot + "/" + topic + "/status";
    return publish(path, data);
}

boolean publishControl(const String& id, const String& topic, const String& state) {
    String path = params.prefix + "/" + id + "/" + topic + "/control";
    return publish(path.c_str(), state.c_str());
}

boolean publishStatus(const String& topic, const String& data) {
    String path = params.deviceRoot + "/" + topic + "/status";
    String json = "{}";
    jsonWriteStr(json, "status", data);
    return publish(path.c_str(), json.c_str());
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