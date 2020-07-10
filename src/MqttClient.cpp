#include "MqttClient.h"

#include <PubSubClient.h>
#include <WiFiClient.h>

#include "Global.h"
#include "Scenario.h"
#include "Logger.h"

static const char* MODULE = "Mqtt";

namespace MqttClient {

WiFiClient espClient;
PubSubClient mqtt(espClient);

String _deviceRoot;
String _addr;
int _port;
String _user;
String _pass;
String _uuid;
String _prefix;

void init() {
    pm.info(TAG_INIT);

    _addr = config.mqtt()->getServer();
    _port = config.mqtt()->getPort();
    _user = config.mqtt()->getUser();
    _pass = config.mqtt()->getPass();
    _prefix = config.mqtt()->getPrefix();

    _uuid = getChipId();
    _deviceRoot = _prefix + "/" + _uuid;
}

void disconnect() {
    pm.info("disconnected");
    mqtt.disconnect();
}

void reconnect() {
    disconnect();
    init();
    connect();
}

void loop() {
    if (mqtt.connected()) {
        mqtt.loop();
    }
}

bool isConnected() {
    return mqtt.connected();
}

void subscribe() {
    pm.info("subscribe: " + _prefix);
    mqtt.setCallback(handleSubscribedUpdates);
    mqtt.subscribe(_prefix.c_str());
    mqtt.subscribe((_deviceRoot + "/+/control").c_str());
    mqtt.subscribe((_deviceRoot + "/order").c_str());
    mqtt.subscribe((_deviceRoot + "/update").c_str());
    mqtt.subscribe((_deviceRoot + "/devc").c_str());
    mqtt.subscribe((_deviceRoot + "/devs").c_str());
}

boolean connect() {
    bool res = false;
    pm.info("connecting " + _addr + ":" + String(_port, DEC));
    mqtt.setServer(_addr.c_str(), _port);
    res = mqtt.connect(_uuid.c_str(), _user.c_str(), _pass.c_str());
    if (res) {
        subscribe();
    } else {
        pm.error("could't connect: " + getStateStr());
    }
    return res;
}

const String parseControl(const String& str) {
    String res;
    String num1 = str.substring(str.length() - 1);
    String num2 = str.substring(str.length() - 2, str.length() - 1);
    if (isDigitStr(num1) && isDigitStr(num2)) {
        res = str.substring(0, str.length() - 2) + "Set" + num2 + num1;
    } else {
        if (isDigitStr(num1)) {
            res = str.substring(0, str.length() - 1) + "Set" + num1;
        }
    }
    return res;
}

void handleSubscribedUpdates(char* topic, uint8_t* payload, size_t length) {
    pm.info("<= " + String(topic));
    String topicStr = String(topic);
    String payloadStr = "";
    payloadStr.reserve(length + 1);
    for (size_t i = 0; i < length; i++) {
        payloadStr += (char)payload[i];
    }
    if (payloadStr.equalsIgnoreCase("hello")) {
        publishWidgets();
        publishState();
        Logger::publish();
    } else if (topicStr.indexOf("control")) {
        // название топика -  команда,
        // значение - параметр
        //IoTmanager/800324-1458415/button99/control 1
        String topic = selectFromMarkerToMarker(topicStr, "/", 3);
        topic = parseControl(topic);
        String number = selectToMarkerLast(topic, "Set");
        topic.replace(number, "");
        addOrder(topic + " " + number + " " + payloadStr);
    } else if (topicStr.indexOf("order")) {
        payloadStr.replace("_", " ");
        addOrder(payloadStr);
    } else if (topicStr.indexOf("update")) {
        if (payloadStr == "1") {
            perform_upgrade();
        }
    } else if (topicStr.indexOf("devc")) {
        writeFile(DEVICE_COMMAND_FILE, payloadStr);
        device_init();
    } else if (topicStr.indexOf("devs")) {
        writeFile(DEVICE_SCENARIO_FILE, payloadStr);
        Scenario::reinit();
    }
}

boolean publish(const String& topic, const String& data) {
    if (!mqtt.connected()) {
        return false;
    }
    if (mqtt.beginPublish(topic.c_str(), data.length(), false)) {
        mqtt.print(data);
        return mqtt.endPublish();
    }
    pm.error("on publish " + topic + " " + data);
    return false;
}

boolean publishData(const String& topic, const String& data) {
    String path = _deviceRoot + "/" + topic;
    return publish(path, data);
}

boolean publishChart(const String& topic, const String& data) {
    String path = _deviceRoot + "/" + topic + "/status";
    return publish(path, data);
}

boolean publishControl(const String& id, const String& topic, const String& state) {
    String path = _prefix + "/" + id + "/" + topic + "/control";
    return publish(path.c_str(), state.c_str());
}

boolean publishStatus(const String& topic, const String& data) {
    String path = _deviceRoot + "/" + topic + "/status";
    String json = "{}";
    jsonWriteStr(json, "status", data);
    return publish(path.c_str(), json.c_str());
}

boolean publishOrder(const String& topic, const String& order) {
    String path = _prefix + "/" + topic + "/order";
    return publish(path.c_str(), order.c_str());
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

void publishState() {
    // берет строку json и ключи превращает в топики а значения колючей в них посылает
    // {"name":"MODULES","lang":"","ip":"192.168.43.60","DS":"34.00","rel1":"1","rel2":"1"}
    // "name":"MODULES","lang":"","ip":"192.168.43.60","DS":"34.00","rel1":"1","rel2":"1"
    // "name":"MODULES","lang":"","ip":"192.168.43.60","DS":"34.00","rel1":"1","rel2":"1",
    String str = liveData.get();
    str.replace("{", "");
    str.replace("}", "");
    str += ",";

    while (str.length()) {
        String tmp = selectToMarker(str, ",");

        String topic = selectToMarker(tmp, ":");
        topic.replace("\"", "");

        String state = selectToMarkerLast(tmp, ":");
        state.replace("\"", "");

        if ((topic != "time") && (topic != "name") && (topic != "lang") && (topic != "ip") && (topic.indexOf("_in") < 0)) {
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