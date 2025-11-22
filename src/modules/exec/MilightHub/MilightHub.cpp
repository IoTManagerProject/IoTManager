#include "Global.h"
#include "classes/IoTItem.h"
#include "utils/SerialPrint.h"
#include "MqttClient.h"

#include <math.h>

namespace {
const char* kTag = "MilightHub";

String escapeJson(const String& input) {
    String escaped;
    escaped.reserve(input.length() + 4);

    for (size_t i = 0; i < input.length(); ++i) {
        const char c = input[i];
        switch (c) {
            case '"':
                escaped += F("\\\"");
                break;
            case '\\':
                escaped += F("\\\\");
                break;
            case '\b':
                escaped += F("\\b");
                break;
            case '\f':
                escaped += F("\\f");
                break;
            case '\n':
                escaped += F("\\n");
                break;
            case '\r':
                escaped += F("\\r");
                break;
            case '\t':
                escaped += F("\\t");
                break;
            default: {
                const uint8_t code = static_cast<uint8_t>(c);
                if (code < 0x20) {
                    char buffer[7];
                    snprintf(buffer, sizeof(buffer), "\\u%04X", code);
                    escaped += buffer;
                } else {
                    escaped += c;
                }
                break;
            }
        }
    }

    return escaped;
}

bool isNumericString(const String& value) {
    if (!value.length()) {
        return false;
    }

    bool seenDigit = false;
    bool seenDot = false;
    size_t idx = 0;

    if (value[0] == '-' || value[0] == '+') {
        idx = 1;
    }

    for (; idx < value.length(); ++idx) {
        const char c = value[idx];
        if (c == '.') {
            if (seenDot) {
                return false;
            }
            seenDot = true;
            continue;
        }
        if (!isDigit(c)) {
            return false;
        }
        seenDigit = true;
    }

    return seenDigit;
}

String encodeUriComponent(const String& value) {
    static const char* hex = "0123456789ABCDEF";
    String encoded;
    encoded.reserve(value.length() * 3);

    for (size_t i = 0; i < value.length(); ++i) {
        const uint8_t c = static_cast<uint8_t>(value[i]);
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' || c == '_' ||
            c == '.' || c == '~') {
            encoded += static_cast<char>(c);
        } else {
            encoded += '%';
            encoded += hex[(c >> 4) & 0x0F];
            encoded += hex[c & 0x0F];
        }
    }

    return encoded;
}

String doubleToString(float value) {
    if (isnan(value) || isinf(value)) {
        return String();
    }

    const long whole = static_cast<long>(value);
    if (fabs(value - static_cast<float>(whole)) < 0.0005f) {
        return String(whole);
    }

    return String(value, 3);
}

String normalizeBoolString(String value) {
    value.trim();
    value.toLowerCase();
    return value;
}

String toParamString(const IoTValue& param) {
    if (param.isDecimal) {
        return doubleToString(param.valD);
    }

    String copy = param.valS;
    copy.trim();
    return copy;
}
}

class MilightHubItem : public IoTItem {
public:
    explicit MilightHubItem(String parameters);

    IoTValue execute(String command, std::vector<IoTValue>& param) override;
    void loop() override;
    void onMqttRecive(String& topic, String& msg) override;
    String getMqttExterSub() override;

private:
    struct RequestContext {
        bool valid = false;
        bool byAlias = true;
        String alias;
        String deviceId;
        String remoteType;
        int groupId = 0;
        bool blockOnQueue = false;
        bool normalized = false;
    };

private:
    RequestContext currentContext() const;
    String buildTargetPath(const RequestContext& ctx) const;
    String buildUrl(const String& path, bool block, bool normalized) const;
    bool performRequest(const String& method, const RequestContext& ctx, const String& payload, String& response, int& statusCode);
    void storeResponse(const String& payload, int statusCode, bool notify);
    bool sendUpdate(const String& payload);
    bool sendFieldUpdate(const String& field, const IoTValue& value);
    bool sendCommandPayload(const String& payload);
    bool sendCommandsPayload(const String& payload);
    bool requestState();
    bool deleteState();
    bool postRawCommand(const String& payload, const String& remoteTypeOverride);

    String valueToJsonLiteral(const IoTValue& value) const;
    String buildFieldPayload(const String& field, const IoTValue& value) const;
    String buildCommandPayload(const String& command) const;
    String buildCommandsPayload(const String& commands) const;

    void setAliasInternal(const String& alias);
    void setDeviceInternal(const String& deviceId, const String& remoteType, int groupId);
    void ensureMqttSubscription();
    void handleMqttMessage(String payload);
    bool handleJsonMqttCommand(JsonVariantConst root);
    void handlePlainMqttCommand(const String& payload);
    IoTValue jsonVariantToIoTValue(JsonVariantConst variant) const;
    IoTValue makeStringValue(const String& text) const;
    void publishMqttResponse(const String& payload, int statusCode);
    String resolveTopic(const String& topic, bool addPrefix) const;

private:
    String _host;
    uint16_t _port = 80;
    bool _useAlias = true;
    String _alias;
    String _deviceId;
    String _remoteType;
    int _groupId = 0;
    bool _blockOnQueue = false;
    bool _normalizedResponse = false;
    bool _rememberResponse = true;
    String _username;
    String _password;
    uint16_t _timeoutMs = 5000;
    String _mqttCommandTopic;
    bool _mqttCommandAddPrefix = false;
    String _mqttResponseTopic;
    bool _mqttResponseAddPrefix = false;
    String _resolvedCommandTopic;
    String _resolvedResponseTopic;
    bool _mqttSubscribed = false;
};

MilightHubItem::MilightHubItem(String parameters) : IoTItem(parameters) {
    _host = jsonReadStr(parameters, F("host"));
    if (_host.length() == 0) {
        SerialPrint("W", kTag, F("host not set"));
    }

    const int port = jsonReadInt(parameters, F("port"));
    if (port > 0 && port <= 65535) {
        _port = static_cast<uint16_t>(port);
    }

    String targetType = jsonReadStr(parameters, F("targetType"));
    targetType.toLowerCase();
    if (targetType == F("device")) {
        _useAlias = false;
    }

    _alias = jsonReadStr(parameters, F("alias"));
    _deviceId = jsonReadStr(parameters, F("deviceId"));
    _remoteType = jsonReadStr(parameters, F("remoteType"));
    _groupId = jsonReadInt(parameters, F("groupId"));
    if (_groupId < 0) {
        _groupId = 0;
    }

    _blockOnQueue = jsonReadBool(parameters, F("blockOnQueue"));
    _normalizedResponse = jsonReadBool(parameters, F("normalizedResponse"));

    const String remember = jsonReadStr(parameters, F("storeResponse"));
    if (remember.length()) {
        String normalized = remember;
        normalized.toLowerCase();
        _rememberResponse = (normalized == F("true"));
    } else {
        _rememberResponse = jsonReadBool(parameters, F("storeResponse"));
    }

    _username = jsonReadStr(parameters, F("username"));
    _password = jsonReadStr(parameters, F("password"));

    const int timeout = jsonReadInt(parameters, F("timeout"));
    if (timeout > 0) {
        _timeoutMs = static_cast<uint16_t>(timeout);
    }

    _mqttCommandTopic = jsonReadStr(parameters, F("mqttCommandTopic"));
    _mqttCommandAddPrefix = true;
    jsonRead(parameters, F("mqttCommandUsePrefix"), _mqttCommandAddPrefix, false);
    _mqttResponseTopic = jsonReadStr(parameters, F("mqttResponseTopic"));
    _mqttResponseAddPrefix = true;
    jsonRead(parameters, F("mqttResponseUsePrefix"), _mqttResponseAddPrefix, false);
    _resolvedCommandTopic = resolveTopic(_mqttCommandTopic, _mqttCommandAddPrefix);
    _resolvedResponseTopic = resolveTopic(_mqttResponseTopic, _mqttResponseAddPrefix);
    if (_resolvedCommandTopic.length() && mqttIsConnect()) {
        mqttSubscribeExternal(_mqttCommandTopic, _mqttCommandAddPrefix);
        _mqttSubscribed = true;
    }

    value.isDecimal = false;
}

void MilightHubItem::loop() {
    ensureMqttSubscription();
    IoTItem::loop();
}

MilightHubItem::RequestContext MilightHubItem::currentContext() const {
    RequestContext ctx;
    ctx.blockOnQueue = _blockOnQueue;
    ctx.normalized = _normalizedResponse;

    if (_useAlias) {
        ctx.byAlias = true;
        ctx.alias = _alias;
        ctx.valid = ctx.alias.length() > 0;
    } else {
        ctx.byAlias = false;
        ctx.deviceId = _deviceId;
        ctx.remoteType = _remoteType;
        ctx.groupId = _groupId;
        ctx.valid = ctx.deviceId.length() > 0 && ctx.remoteType.length() > 0;
    }

    return ctx;
}

String MilightHubItem::buildTargetPath(const RequestContext& ctx) const {
    if (!ctx.valid) {
        return String();
    }

    if (ctx.byAlias) {
        return String(F("/gateways/")) + encodeUriComponent(ctx.alias);
    }

    String path(F("/gateways/"));
    path += encodeUriComponent(ctx.deviceId);
    path += '/';
    path += encodeUriComponent(ctx.remoteType);
    path += '/';
    path += String(ctx.groupId);
    return path;
}

String MilightHubItem::buildUrl(const String& path, bool block, bool normalized) const {
    String url(F("http://"));
    url += _host;
    if (_port != 80) {
        url += ':';
        url += String(_port);
    }
    url += path;

    char separator = '?';
    if (block) {
        url += separator;
        url += F("blockOnQueue=true");
        separator = '&';
    }
    if (normalized) {
        url += separator;
        url += F("fmt=normalized");
    }
    return url;
}

void MilightHubItem::ensureMqttSubscription() {
    if (!_resolvedCommandTopic.length()) {
        return;
    }
    if (!mqttIsConnect()) {
        _mqttSubscribed = false;
        return;
    }
    if (_mqttSubscribed) {
        return;
    }
    mqttSubscribeExternal(_mqttCommandTopic, _mqttCommandAddPrefix);
    _mqttSubscribed = true;
}

bool MilightHubItem::performRequest(const String& method, const RequestContext& ctx, const String& payload, String& response, int& statusCode) {
    if (!ctx.valid) {
        SerialPrint("E", kTag, F("target not configured"));
        return false;
    }

    const String path = buildTargetPath(ctx);
    const String url = buildUrl(path, ctx.blockOnQueue, ctx.normalized);

    WiFiClient client;
    HTTPClient http;

#if defined(ESP8266) || defined(ESP32) || defined(LIBRETINY)
    if (!http.begin(client, url)) {
#else
    if (!http.begin(url)) {
#endif
        SerialPrint("E", kTag, String(F("HTTP begin failed: ")) + url);
        return false;
    }

    if (_username.length()) {
        http.setAuthorization(_username.c_str(), _password.c_str());
    }

    if (_timeoutMs > 0) {
        http.setTimeout(_timeoutMs);
    }

    if (method == F("PUT") || method == F("POST")) {
        http.addHeader(F("Content-Type"), F("application/json"));
    }

    if (method == F("GET")) {
        statusCode = http.GET();
    } else if (method == F("PUT")) {
        statusCode = http.PUT(payload);
    } else if (method == F("POST")) {
        statusCode = http.POST(payload);
    } else if (method == F("DELETE")) {
        statusCode = http.sendRequest(F("DELETE"));
    } else {
        SerialPrint("E", kTag, String(F("unsupported method: ")) + method);
        http.end();
        return false;
    }

    response = http.getString();
    http.end();

    if (statusCode < 0) {
        SerialPrint("E", kTag, String(F("HTTP error: ")) + HTTPClient::errorToString(statusCode));
        return false;
    }

    const bool success = statusCode >= 200 && statusCode < 300;
    SerialPrint(success ? "i" : "E", kTag, method + ' ' + url + F(" -> ") + String(statusCode));

    if (response.length()) {
        SerialPrint("i", kTag, String(F("response: ")) + response);
    }

    return success;
}

void MilightHubItem::storeResponse(const String& payload, int statusCode, bool notify) {
    value.isDecimal = false;
    if (payload.length()) {
        value.valS = payload;
    } else {
        value.valS = String(statusCode);
    }

    publishMqttResponse(payload, statusCode);

    if (notify) {
        regEvent(value.valS, kTag);
    }
}

bool MilightHubItem::sendUpdate(const String& rawPayload) {
    if (!isNetworkActive()) {
        SerialPrint("W", kTag, F("network offline"));
        return false;
    }

    String payload = rawPayload;
    payload.trim();
    if (!payload.length()) {
        return false;
    }

    if (!(payload.startsWith("{") || payload.startsWith("["))) {
        String normalized = payload;
        normalized.trim();
        normalized.toUpperCase();
        if (normalized == F("ON") || normalized == F("OFF")) {
            payload = String(F("{\"state\":\"")) + normalized + F("\"}");
        } else {
            SerialPrint("E", kTag, F("set expects JSON payload or ON/OFF"));
            return false;
        }
    }

    RequestContext ctx = currentContext();
    String response;
    int status = 0;
    const bool ok = performRequest(F("PUT"), ctx, payload, response, status);
    storeResponse(response, status, _rememberResponse);
    return ok;
}

bool MilightHubItem::sendFieldUpdate(const String& field, const IoTValue& fieldValue) {
    if (!field.length()) {
        return false;
    }

    const String payload = buildFieldPayload(field, fieldValue);
    return sendUpdate(payload);
}

bool MilightHubItem::sendCommandPayload(const String& payload) {
    if (!payload.length()) {
        return false;
    }

    RequestContext ctx = currentContext();
    String response;
    int status = 0;
    const bool ok = performRequest(F("PUT"), ctx, payload, response, status);
    storeResponse(response, status, _rememberResponse);
    return ok;
}

bool MilightHubItem::sendCommandsPayload(const String& payload) {
    return sendCommandPayload(payload);
}

bool MilightHubItem::requestState() {
    if (!isNetworkActive()) {
        SerialPrint("W", kTag, F("network offline"));
        return false;
    }

    RequestContext ctx = currentContext();
    ctx.blockOnQueue = false;  // reading does not need block
    String response;
    int status = 0;
    const bool ok = performRequest(F("GET"), ctx, String(), response, status);
    storeResponse(response, status, true);
    return ok;
}

bool MilightHubItem::deleteState() {
    if (!isNetworkActive()) {
        SerialPrint("W", kTag, F("network offline"));
        return false;
    }

    RequestContext ctx = currentContext();
    String response;
    int status = 0;
    const bool ok = performRequest(F("DELETE"), ctx, String(), response, status);
    storeResponse(response, status, _rememberResponse);
    return ok;
}

bool MilightHubItem::postRawCommand(const String& payload, const String& remoteTypeOverride) {
    if (!isNetworkActive()) {
        SerialPrint("W", kTag, F("network offline"));
        return false;
    }

    String remote = remoteTypeOverride.length() ? remoteTypeOverride : _remoteType;
    if (!remote.length()) {
        SerialPrint("E", kTag, F("remoteType required for raw command"));
        return false;
    }

    RequestContext ctx = currentContext();
    ctx.valid = true;
    ctx.byAlias = false;
    ctx.deviceId = String();
    ctx.remoteType = remote;
    ctx.groupId = 0;

    const String path = String(F("/raw_commands/")) + encodeUriComponent(remote);
    const String url = buildUrl(path, false, false);

    WiFiClient client;
    HTTPClient http;

#if defined(ESP8266) || defined(ESP32) || defined(LIBRETINY)
    if (!http.begin(client, url)) {
#else
    if (!http.begin(url)) {
#endif
        SerialPrint("E", kTag, String(F("HTTP begin failed: ")) + url);
        return false;
    }

    if (_username.length()) {
        http.setAuthorization(_username.c_str(), _password.c_str());
    }
    if (_timeoutMs > 0) {
        http.setTimeout(_timeoutMs);
    }
    http.addHeader(F("Content-Type"), F("application/json"));

    const int status = http.POST(payload);
    const String response = http.getString();
    http.end();

    if (status < 0) {
        SerialPrint("E", kTag, String(F("HTTP error: ")) + HTTPClient::errorToString(status));
        return false;
    }

    const bool ok = status >= 200 && status < 300;
    SerialPrint(ok ? "i" : "E", kTag, String(F("POST ")) + url + F(" -> ") + String(status));
    if (response.length()) {
        SerialPrint("i", kTag, String(F("response: ")) + response);
    }

    storeResponse(response, status, _rememberResponse);
    return ok;
}

String MilightHubItem::getMqttExterSub() {
    return _resolvedCommandTopic;
}

void MilightHubItem::onMqttRecive(String& topic, String& msg) {
    if (!_resolvedCommandTopic.length()) {
        return;
    }
    if (topic != _resolvedCommandTopic) {
        return;
    }
    if (msg.startsWith(F("HELLO"))) {
        return;
    }
    handleMqttMessage(msg);
}

String MilightHubItem::valueToJsonLiteral(const IoTValue& value) const {
    if (value.isDecimal) {
        return doubleToString(value.valD);
    }

    String text = value.valS;
    text.trim();
    if (!text.length()) {
        return String(F("\"\""));
    }

    String lower = text;
    lower.toLowerCase();
    if (lower == F("true") || lower == F("false")) {
        return lower;
    }

    if (isNumericString(text)) {
        return text;
    }

    if (text.startsWith("{") || text.startsWith("[") ||
        (text.startsWith("\"") && text.endsWith("\""))) {
        return text;
    }

    return String('"') + escapeJson(text) + '"';
}

String MilightHubItem::buildFieldPayload(const String& rawField, const IoTValue& value) const {
    String field = rawField;
    field.trim();
    if (!field.length()) {
        return String();
    }

    String payload(F("{\""));
    payload += field;
    payload += F("\":");
    payload += valueToJsonLiteral(value);
    payload += '}';
    return payload;
}

String MilightHubItem::buildCommandPayload(const String& command) const {
    String trimmed = command;
    trimmed.trim();
    if (!trimmed.length()) {
        return String();
    }

    if (trimmed.startsWith("{")) {
        return trimmed;
    }

    if (trimmed.startsWith("\"") && trimmed.endsWith("\"")) {
        return String(F("{\"command\":")) + trimmed + '}';
    }

    return String(F("{\"command\":\"")) + escapeJson(trimmed) + F("\"}");
}

String MilightHubItem::buildCommandsPayload(const String& commands) const {
    String trimmed = commands;
    trimmed.trim();
    if (!trimmed.length()) {
        return String();
    }

    if (trimmed.startsWith("[")) {
        return String(F("{\"commands\":")) + trimmed + '}';
    }

    if (trimmed.startsWith("{")) {
        return trimmed;
    }

    return String(F("{\"commands\":[\"")) + escapeJson(trimmed) + F("\"]}"));
}

void MilightHubItem::setAliasInternal(const String& alias) {
    if (!alias.length()) {
        SerialPrint("W", kTag, F("empty alias ignored"));
        return;
    }
    _alias = alias;
    _useAlias = true;
    SerialPrint("i", kTag, String(F("alias set to ")) + _alias);
}

void MilightHubItem::setDeviceInternal(const String& deviceId, const String& remoteType, int groupId) {
    if (!deviceId.length() || !remoteType.length()) {
        SerialPrint("E", kTag, F("deviceId and remoteType required"));
        return;
    }
    _deviceId = deviceId;
    _remoteType = remoteType;
    if (groupId < 0) {
        groupId = 0;
    }
    _groupId = groupId;
    _useAlias = false;
    SerialPrint("i", kTag, String(F("device target: ")) + _deviceId + F(" / ") + _remoteType + F(" / ") + String(_groupId));
}

void MilightHubItem::handleMqttMessage(String payload) {
    payload.trim();
    if (!payload.length()) {
        return;
    }

    const size_t capacity = payload.length() + 64;
    DynamicJsonDocument doc(capacity);
    const DeserializationError err = deserializeJson(doc, payload);
    if (!err) {
        JsonVariantConst root = doc.as<JsonVariantConst>();
        if (handleJsonMqttCommand(root)) {
            return;
        }
    }

    handlePlainMqttCommand(payload);
}

bool MilightHubItem::handleJsonMqttCommand(JsonVariantConst root) {
    if (root.is<JsonObjectConst>()) {
        JsonObjectConst obj = root.as<JsonObjectConst>();
        String command = obj[F("command")].as<String>();
        if (!command.length()) {
            command = obj[F("action")].as<String>();
        }

        if (!command.length()) {
            String serialized;
            serializeJson(obj, serialized);
            sendUpdate(serialized);
            return true;
        }

        std::vector<IoTValue> params;
        if (obj.containsKey(F("params")) && obj[F("params")].is<JsonArrayConst>()) {
            JsonArrayConst arr = obj[F("params")].as<JsonArrayConst>();
            for (JsonVariantConst entry : arr) {
                params.push_back(jsonVariantToIoTValue(entry));
            }
        } else {
            if (obj.containsKey(F("payload"))) {
                params.push_back(jsonVariantToIoTValue(obj[F("payload")]));
            } else if (obj.containsKey(F("value"))) {
                params.push_back(jsonVariantToIoTValue(obj[F("value")]));
            }
        }

        if (command.equalsIgnoreCase(F("setdevice")) &&
            obj.containsKey(F("deviceId")) && obj.containsKey(F("remoteType")) && obj.containsKey(F("groupId"))) {
            params.clear();
            params.push_back(makeStringValue(obj[F("deviceId")].as<String>()));
            params.push_back(makeStringValue(obj[F("remoteType")].as<String>()));
            params.push_back(makeStringValue(obj[F("groupId")].as<String>()));
        }

        if (command.equalsIgnoreCase(F("setfield")) && obj.containsKey(F("field")) && obj.containsKey(F("value"))) {
            params.clear();
            params.push_back(makeStringValue(obj[F("field")].as<String>()));
            params.push_back(jsonVariantToIoTValue(obj[F("value")]));
        } else if (obj.containsKey(F("field"))) {
            params.insert(params.begin(), makeStringValue(obj[F("field")].as<String>()));
        }

        if (command.equalsIgnoreCase(F("raw")) && obj.containsKey(F("remoteType")) && !params.empty()) {
            params.push_back(makeStringValue(obj[F("remoteType")].as<String>()));
        }

        execute(command, params);
        return true;
    }

    if (root.is<JsonArrayConst>()) {
        String serialized;
        serializeJson(root, serialized);
        sendUpdate(serialized);
        return true;
    }

    return false;
}

void MilightHubItem::handlePlainMqttCommand(const String& payload) {
    String lowered = payload;
    lowered.toLowerCase();

    if (lowered == F("get")) {
        requestState();
        return;
    }
    if (lowered == F("deletestate") || lowered == F("delete")) {
        deleteState();
        return;
    }
    if (lowered.startsWith(F("command "))) {
        String commandPayload = payload.substring(8);
        const String payloadJson = buildCommandPayload(commandPayload);
        sendCommandPayload(payloadJson);
        return;
    }
    if (lowered.startsWith(F("raw "))) {
        String rawPayload = payload.substring(4);
        postRawCommand(rawPayload, String());
        return;
    }

    sendUpdate(payload);
}

IoTValue MilightHubItem::jsonVariantToIoTValue(JsonVariantConst variant) const {
    IoTValue result;
    if (variant.is<float>() || variant.is<double>() || variant.is<int>() || variant.is<long>()) {
        result.isDecimal = true;
        result.valD = static_cast<float>(variant.as<double>());
        result.valS = doubleToString(result.valD);
        return result;
    }

    if (variant.is<bool>()) {
        result.isDecimal = false;
        result.valS = variant.as<bool>() ? F("true") : F("false");
        return result;
    }

    if (variant.is<const char*>()) {
        result.isDecimal = false;
        result.valS = variant.as<String>();
        return result;
    }

    result.isDecimal = false;
    String serialized;
    serializeJson(variant, serialized);
    result.valS = serialized;
    return result;
}

IoTValue MilightHubItem::makeStringValue(const String& text) const {
    IoTValue result;
    result.isDecimal = false;
    result.valS = text;
    return result;
}

void MilightHubItem::publishMqttResponse(const String& payload, int statusCode) {
    if (!_resolvedResponseTopic.length()) {
        return;
    }
    if (!mqttIsConnect()) {
        return;
    }

    String json(F("{}"));
    jsonWriteInt_(json, F("status"), statusCode);
    if (payload.length()) {
        jsonWriteStr_(json, F("payload"), payload);
    }

    if (!publish(_resolvedResponseTopic, json)) {
        SerialPrint("W", kTag, F("MQTT response publish failed"));
    }
}

String MilightHubItem::resolveTopic(const String& topic, bool addPrefix) const {
    if (!topic.length()) {
        return String();
    }
    if (!addPrefix) {
        return topic;
    }
    return mqttPrefix + '/' + topic;
}

IoTValue MilightHubItem::execute(String command, std::vector<IoTValue>& param) {
    IoTValue result;

    command.toLowerCase();

    if (command == F("set")) {
        if (!param.empty()) {
            sendUpdate(toParamString(param[0]));
        }
    } else if (command == F("setfield")) {
        if (param.size() >= 2) {
            const String field = toParamString(param[0]);
            sendFieldUpdate(field, param[1]);
        }
    } else if (command == F("command")) {
        if (!param.empty()) {
            const String payload = buildCommandPayload(toParamString(param[0]));
            sendCommandPayload(payload);
        }
    } else if (command == F("commands")) {
        if (!param.empty()) {
            const String payload = buildCommandsPayload(toParamString(param[0]));
            sendCommandsPayload(payload);
        }
    } else if (command == F("get")) {
        requestState();
    } else if (command == F("deletestate")) {
        deleteState();
    } else if (command == F("setalias")) {
        if (!param.empty()) {
            setAliasInternal(toParamString(param[0]));
        }
    } else if (command == F("setdevice")) {
        if (param.size() >= 3) {
            const String deviceId = toParamString(param[0]);
            const String remoteType = toParamString(param[1]);
            const String groupString = toParamString(param[2]);
            int group = groupString.toInt();
            setDeviceInternal(deviceId, remoteType, group);
        }
    } else if (command == F("setblock")) {
        if (!param.empty()) {
            String value = toParamString(param[0]);
            value = normalizeBoolString(value);
            if (value == F("true") || value == F("1")) {
                _blockOnQueue = true;
            } else if (value == F("false") || value == F("0")) {
                _blockOnQueue = false;
            }
            SerialPrint("i", kTag, String(F("blockOnQueue=")) + (_blockOnQueue ? F("true") : F("false")));
        }
    } else if (command == F("setnormalized")) {
        if (!param.empty()) {
            String value = toParamString(param[0]);
            value = normalizeBoolString(value);
            if (value == F("true") || value == F("1")) {
                _normalizedResponse = true;
            } else if (value == F("false") || value == F("0")) {
                _normalizedResponse = false;
            }
            SerialPrint("i", kTag, String(F("normalizedResponse=")) + (_normalizedResponse ? F("true") : F("false")));
        }
    } else if (command == F("setstore")) {
        if (!param.empty()) {
            String value = toParamString(param[0]);
            value = normalizeBoolString(value);
            if (value == F("true") || value == F("1")) {
                _rememberResponse = true;
            } else if (value == F("false") || value == F("0")) {
                _rememberResponse = false;
            }
            SerialPrint("i", kTag, String(F("storeResponse=")) + (_rememberResponse ? F("true") : F("false")));
        }
    } else if (command == F("setauth")) {
        if (param.size() >= 2) {
            _username = toParamString(param[0]);
            _password = toParamString(param[1]);
            SerialPrint("i", kTag, F("basic auth credentials updated"));
        }
    } else if (command == F("settimeout")) {
        if (!param.empty()) {
            const int timeout = toParamString(param[0]).toInt();
            if (timeout > 0) {
                _timeoutMs = static_cast<uint16_t>(timeout);
                SerialPrint("i", kTag, String(F("timeout=")) + String(_timeoutMs));
            }
        }
    } else if (command == F("sethost")) {
        if (!param.empty()) {
            _host = toParamString(param[0]);
            SerialPrint("i", kTag, String(F("host=")) + _host);
        }
    } else if (command == F("setport")) {
        if (!param.empty()) {
            const int newPort = toParamString(param[0]).toInt();
            if (newPort > 0 && newPort <= 65535) {
                _port = static_cast<uint16_t>(newPort);
                SerialPrint("i", kTag, String(F("port=")) + String(_port));
            }
        }
    } else if (command == F("raw")) {
        if (!param.empty()) {
            const String payload = toParamString(param[0]);
            String remoteOverride;
            if (param.size() >= 2) {
                remoteOverride = toParamString(param[1]);
            }
            postRawCommand(payload, remoteOverride);
        }
    }

    return result;
}

void* getAPI_MilightHub(String subtype, String param) {
    if (subtype == F("MilightHub")) {
        return new MilightHubItem(param);
    }
    return nullptr;
}
