#include "MqttClient.h"

#include <PubSubClient.h>
#include <WiFiClient.h>

#include "Global.h"
#include "Scenario.h"
#include "Logger.h"

static const char* MODULE = "Mqtt";

namespace MqttClient {

WiFiClient espClient;
PubSubClient _mqtt(espClient);
MqttWriter* _mqttWriter;

static unsigned long RECONNECT_INTERVAL = 5000;
static size_t RECONNECT_ATTEMPTS_MAX = 10;
unsigned long _lastConnetionAttempt = 0;
size_t _connectionAttempts = 0;

String _deviceRoot;
String _addr;
int _port;
String _user;
String _pass;
String _uuid;
String _prefix;

MqttWriter* getWriter(const char* topic) {
    String path = _deviceRoot + "/" + topic;
    return new MqttWriter(&_mqtt, path.c_str());
}

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

bool connect() {
    bool res = false;
    _mqtt.setServer(_addr.c_str(), _port);
    res = _mqtt.connect(_uuid.c_str(), _user.c_str(), _pass.c_str());
    if (res) {
        pm.info("connected " + _addr + ":" + String(_port, DEC));
    } else {
        pm.error("could't connect: " + getStateStr());
    }
    return res;
}

void disconnect() {
    pm.info("disconnected");
    _mqtt.disconnect();
}

void reconnect() {
    if (_mqtt.connected()) {
        _mqtt.disconnect();
    }
    init();
    if (connect()) {
        subscribe();
    }
}

void subscribe() {
    pm.info("subscribe: " + _prefix);
    _mqtt.setCallback(handleSubscribedUpdates);
    _mqtt.subscribe(_prefix.c_str());
    _mqtt.subscribe((_deviceRoot + "/+/control").c_str());
    _mqtt.subscribe((_deviceRoot + "/order").c_str());
    _mqtt.subscribe((_deviceRoot + "/update").c_str());
    _mqtt.subscribe((_deviceRoot + "/devc").c_str());
    _mqtt.subscribe((_deviceRoot + "/devs").c_str());
}

bool isConnected() {
    return _mqtt.connected();
}

bool hasAttempts() {
    return RECONNECT_ATTEMPTS_MAX ? _connectionAttempts < RECONNECT_ATTEMPTS_MAX : true;
}

void loop() {
    if (!config.mqtt()->isEnabled()) {
        if (isConnected()) {
            disconnect();
        }
        return;
    }
    if (!isConnected()) {
        if (!hasAttempts()) {
            return;
        }
        if (millis_since(_lastConnetionAttempt) < RECONNECT_INTERVAL) {
            return;
        }
        reconnect();
        _lastConnetionAttempt = millis();
        if (!isConnected()) {
            _connectionAttempts++;
            if (!hasAttempts()) {
                config.mqtt()->enable(false);
            }
            return;
        }
    }

    _mqtt.loop();
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

const String getStatusPath(const String& name) {
    String res = _deviceRoot;
    res += "/";
    res += name.c_str();
    res += "/status";
    return res;
}

bool publishChartEntry(const char* name, String entry) {
    String payload = "{\"status\":";
    payload += entry;
    payload += "}";
    return mqtt_publish(getStatusPath(name).c_str(), payload.c_str());
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
        publishCharts();
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

boolean mqtt_publish(const String& topic, const String& data) {
    if (!_mqtt.connected()) {
        pm.error("not connected " + topic + " " + data);
        return false;
    }
    if (_mqtt.beginPublish(topic.c_str(), data.length(), false)) {
        _mqtt.print(data);
        return _mqtt.endPublish();
    }
    pm.error("beginPublish " + topic + " " + data);
    return false;
}

boolean publishData(const String& topic, const String& data) {
    String path = _deviceRoot + "/" + topic;
    return mqtt_publish(path, data);
}

boolean publishChart(const String& topic, const String& data) {
    String path = _deviceRoot + "/" + topic + "/status";
    return mqtt_publish(path, data);
}

boolean publishControl(const String& id, const String& topic, const String& data) {
    String path = _prefix + "/" + id + "/" + topic + "/control";
    return mqtt_publish(path, data);
}

const String getOrderPath(const String& topic) {
    String res = _prefix;
    res += "/";
    res += topic;
    res += "/order";
    return res;
}

boolean publishStatus(const String& name, const String& value, const ValueType_t type) {
    String status = "{\"status\":";
    if (type == VT_STRING) {
        status += "\"";
    }
    status += value;
    if (type == VT_STRING) {
        status += "\"";
    }
    status += "}";

    return mqtt_publish(getStatusPath(name).c_str(), status);
}

boolean publishOrder(const String& topic, const String& order) {
    return mqtt_publish(getOrderPath(topic), order);
}

const String getStateStr() {
    switch (_mqtt.state()) {
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