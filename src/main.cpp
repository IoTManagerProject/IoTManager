#include "Global.h"

#include "HttpServer.h"
#include "Bus/I2CScanner.h"
#include "Bus/DallasScanner.h"
#include "TickerScheduler/Metric.h"

void not_async_actions();

static const char* MODULE = "Main";

enum LoopItems {
    LI_CLOCK,
    LI_NOT_ASYNC,
    LI_MQTT_CLIENT,
    LI_CMD,
    LI_BUTTON,
    LI_SCENARIO,
    LI_UDP,
    LI_SERIAL,
    LI_TICKETS
};

Metric m;
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
    update_init();

    pm.info(String("HttpServer"));
    HttpServer::init();

    pm.info(String("WebAdmin"));
    web_init();

#ifdef UDP_ENABLED
    pm.info(String("Broadcast"));
    udp_init();
#endif

    ts.add(
        SYS_STAT, 1000 * 60, [&](void*) {
            pm.info(getMemoryStatus());
            pm.info("Loop: ");
            m.print(Serial);

            pm.info("Tickets: ");
            ts.printMetric(Serial);

            m.reset();
            ts.resetMetric();
        },
        nullptr, false);

    just_load = false;
    initialized = true;
}

void loop() {
    if (!initialized) {
        return;
    }
    m.loop();
    timeNow->loop();
    m.add(LI_CLOCK);

#ifdef OTA_UPDATES_ENABLED
    ArduinoOTA.handle();
#endif
#ifdef WS_enable
    ws.cleanupClients();
#endif
    not_async_actions();
    m.add(LI_NOT_ASYNC);

    MqttClient::loop();
    m.add(LI_MQTT_CLIENT);

    loopCmd();
    m.add(LI_CMD);

    loop_button();
    m.add(LI_BUTTON);

    if (configSetup.isScenarioEnabled()) {
        Scenario::process(&events);
        m.add(LI_SCENARIO);
    }

#ifdef UDP_ENABLED
    loopUdp();
#endif
    m.add(LI_UDP);

    loop_serial();
    m.add(LI_SERIAL);

    ts.update();
    m.add(LI_TICKETS);
}

void not_async_actions() {
    if (mqttParamsChanged) {
        MqttClient::reconnect();
        mqttParamsChanged = false;
    }

    do_check_update();

    do_update();

#ifdef UDP_ENABLED
    do_udp_data_parse();
    do_mqtt_send_settings_to_udp();
#endif

    do_scan_bus();
}



void setChipId() {
    chipId = getChipId();
    pm.info("id: " + chipId);
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
