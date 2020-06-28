#include "Global.h"

#include "HttpServer.h"
#include "Bus/I2CScanner.h"
#include "Bus/DallasScanner.h"
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

    pm.info(String("FS"));
    fileSystemInit();

    pm.info(String("Config"));
    loadConfig();

    pm.info(String("Clock"));
    clock_init();

    pm.info(String("Commands"));
    cmd_init();

    pm.info(String("Sensors"));
    sensors_init();

    pm.info(String("Init"));
    all_init();

    pm.info(String("Network"));
    startSTAMode();

    pm.info(String("Uptime"));
    uptime_init();

    if (!TELEMETRY_UPDATE_INTERVAL) {
        pm.info(String("Telemetry: Disabled"));
    }
    telemetry_init();

    pm.info(String("Updater"));
    initUpdater();

    pm.info(String("HttpServer"));
    HttpServer::init();

    pm.info(String("WebAdmin"));
    web_init();

#ifdef UDP_ENABLED
    pm.info(String("Broadcast UDP"));
    udp_init();
#endif
    ts.add(
        TEST, 1000 * 60, [&](void*) {
            pm.info(getMemoryStatus());
        },
        nullptr, true);

    just_load = false;

    initialized = true;
}

void loop() {
    if (!initialized) {
        return;
    }
    timeNow->loop();

#ifdef OTA_UPDATES_ENABLED
    ArduinoOTA.handle();
#endif
#ifdef WS_enable
    ws.cleanupClients();
#endif
    not_async_actions();

    MqttClient::loop();

    loopCmd();

    loop_button();

    loopScenario();

#ifdef UDP_ENABLED
    loopUdp();
#endif

    loop_serial();

    ts.update();
}

void not_async_actions() {
    if (mqttParamsChanged) {
        MqttClient::reconnect();
        mqttParamsChanged = false;
    }

    getLastVersion();

    do_update();

#ifdef UDP_ENABLED
    do_udp_data_parse();
    do_mqtt_send_settings_to_udp();
#endif

    do_scan_bus();
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
        BusScanner* bus;
        switch (busToScan) {
            case BS_I2C:
                bus = new I2CScanner();
                break;
            case BS_ONE_WIRE:
                bus = new DallasScanner();
                break;
            default:
                pm.error("uknown bus: " + String(busToScan, DEC));
                busScanFlag = false;
                return;
        }
        if (bus) {
            bus->scan(configLiveJson);
        }
        busScanFlag = false;
    }
}
