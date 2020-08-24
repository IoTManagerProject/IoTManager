#include "Bus/BusScannerFactory.h"
#include "Class/AsyncActions.h"
#include "Class/Switch.h"
#include "Cmd.h"
#include "DeviceList.h"
#include "Global.h"
#include "HttpServer.h"
#include "Init.h"
#include "Utils/Timings.h"

void not_async_actions();

static const char* MODULE = "Main";

Timings metric;
boolean initialized = false;

void setup() {
    WiFi.setAutoConnect(false);
    WiFi.persistent(false);

    Serial.begin(115200);
    Serial.flush();
    Serial.println();
    Serial.println("--------------started----------------");

    setChipId();

    pm.info("FS");
    fileSystemInit();

    pm.info("Config");
    loadConfig();

    pm.info("Clock");
    clock_init();

    pm.info("Commands");
    cmd_init();

    pm.info("Sensors");
    sensors_init();

    pm.info("Init");
    all_init();

    pm.info("Network");
    startSTAMode();

    pm.info("Uptime");
    uptime_init();

    if (!TELEMETRY_UPDATE_INTERVAL) {
        pm.info("Telemetry: Disabled");
    }
    telemetry_init();

    pm.info("Updater");
    initUpdater();

    pm.info("HttpServer");
    HttpServer::init();

    pm.info("WebAdmin");
    web_init();

#ifdef UDP_ENABLED
    pm.info("Broadcast UDP");
    udp_init();
#endif

    ts.add(
        TEST, 1000 * 60, [&](void*) { pm.info(printMemoryStatus()); }, nullptr, true);

    just_load = false;
    initialized = true;

    async = new AsyncActions();

    async->setCallback([]() {
        Serial.println("123");
    });

    async->setCallback([](const String str) {
        Serial.println(str);
        return true;
    });
    
}

void loop() {
    if (!initialized) {
        return;
    }
#ifdef OTA_UPDATES_ENABLED
    ArduinoOTA.handle();
#endif
#ifdef WS_enable
    ws.cleanupClients();
#endif
#ifdef UDP_ENABLED
    loopUdp();
#endif
    timeNow->loop();
    async->loop();
    not_async_actions();
    MqttClient::loop();
    loopCmd();
    mySwitch->loop();
    loopScenario();
    loopSerial();
    ts.update();
}

void not_async_actions() {
#ifdef UDP_ENABLED
    do_udp_data_parse();
    do_mqtt_send_settings_to_udp();
#endif
    if (mqttParamsChanged) {
        MqttClient::reconnect();
        mqttParamsChanged = false;
    }

    getLastVersion();

    do_update();
    do_scan_bus();
    do_delElement();
    do_getJsonListFromCsv();
}

String getURL(const String& urls) {
    String res = "";
    HTTPClient http;
    http.begin(urls);
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
        res = http.getString();
    } else {
        res = "error";
    }
    http.end();
    return res;
}

void setChipId() {
    chipId = getChipId();
    pm.info("id: " + chipId);
}

void saveConfig() {
    writeFile(String("config.json"), configSetupJson);
}

void setConfigParam(const char* param, const String& value) {
    pm.info("set " + String(param) + ": " + value);
    jsonWriteStr(configSetupJson, param, value);
    saveConfig();
}

#ifdef ESP8266
void setLedStatus(LedStatus_t status) {
    pinMode(LED_PIN, OUTPUT);
    switch (status) {
        case LED_OFF:
            noTone(LED_PIN);
            digitalWrite(LED_PIN, HIGH);
            break;
        case LED_ON:
            noTone(LED_PIN);
            digitalWrite(LED_PIN, LOW);
            break;
        case LED_SLOW:
            tone(LED_PIN, 1);
            break;
        case LED_FAST:
            tone(LED_PIN, 20);
            break;
        default:
            break;
    }
}
#else
void setLedStatus(LedStatus_t status) {
    pinMode(LED_PIN, OUTPUT);
    switch (status) {
        case LED_OFF:
            digitalWrite(LED_PIN, HIGH);
            break;
        case LED_ON:
            digitalWrite(LED_PIN, LOW);
            break;
        case LED_SLOW:
            break;
        case LED_FAST:
            break;
        default:
            break;
    }
}
#endif

void clock_init() {
    timeNow = new Clock();
    timeNow->setNtpPool(jsonReadStr(configSetupJson, "ntp"));
    timeNow->setTimezone(jsonReadStr(configSetupJson, "timezone").toInt());

    ts.add(
        TIME_SYNC, 30000, [&](void*) {
            timeNow->hasSync();
        },
        nullptr, true);
}

void do_scan_bus() {
    if (busScanFlag) {
        String res = "";
        BusScanner* scanner = BusScannerFactory::get(configSetupJson, busToScan, res);
        scanner->scan();
        jsonWriteStr(configLiveJson, String(scanner->tag()), res);
        busScanFlag = false;
    }
}
