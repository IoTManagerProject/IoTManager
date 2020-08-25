
#include "Class/CallBackTest.h"
#include "Class/NotAsinc.h"
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
    upgradeInit();

    pm.info("HttpServer");
    HttpServer::init();

    pm.info("WebAdmin");
    web_init();

#ifdef UDP_ENABLED
    pm.info("Broadcast UDP");
    udpInit();
#endif

    ts.add(
        TEST, 1000 * 60, [&](void*) {
            pm.info(printMemoryStatus());
        },
        nullptr, true);

    just_load = false;
    initialized = true;

    myNotAsincActions = new NotAsinc(5);
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
    MqttClient::loop();
    loopCmd();
    mySwitch->loop();
    loopScenario();
    loopSerial();
    
    myNotAsincActions->loop();
    ts.update();
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


