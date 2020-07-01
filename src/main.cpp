#include "Global.h"

#include "Broadcast.h"
#include "MqttClient.h"
#include "HttpServer.h"
#include "Bus/I2CScanner.h"
#include "Bus/DallasScanner.h"
#include "TickerScheduler/Metric.h"
#include "Sensors.h"

static const char* MODULE = "Main";

void acync_actions();
void config_backup();
void config_restore();

// Async actions
boolean perform_updates_check = false;
boolean perform_upgrade = false;

boolean broadcast_mqtt_settings_flag = false;
void broadcast_mqtt_settings() {
    pm.info("broadcast mqtt");
    broadcast_mqtt_settings_flag = true;
}

boolean perform_bus_scanning_flag = false;
BusScanner_t perform_bus_scanning_bus;
void perform_bus_scanning(BusScanner_t bus) {
    pm.info("bus scan: " + bus);
    perform_bus_scanning_flag = true;
    perform_bus_scanning_bus = bus;
}

boolean perform_system_restart_flag = false;
void perform_system_restart() {
    pm.info("system restart");
    perform_system_restart_flag = true;
}

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
    acync_actions();
    m.add(LI_NOT_ASYNC);

    MqttClient::loop();
    m.add(LI_MQTT_CLIENT);

    loopCmd();
    m.add(LI_CMD);

    loop_button();
    m.add(LI_BUTTON);

    if (config.general()->isScenarioEnabled()) {
        Scenario::process(&events);
        m.add(LI_SCENARIO);
    }

    if (config.general()->isBroadcastEnabled()) {
        Broadcast::loop();
        m.add(LI_UDP);
    }

    loop_serial();
    m.add(LI_SERIAL);

    ts.update();
    m.add(LI_TICKETS);
}

void acync_actions() {
    if (mqttParamsChangedFlag) {
        MqttClient::reconnect();
        mqttParamsChangedFlag = false;
    }

    if (perform_updates_check) {
        lastVersion = Updater::check();
        jsonWriteStr(configSetupJson, "last_version", lastVersion);
        perform_updates_check = false;
    }

    if (perform_upgrade) {
        config_backup();
        bool res = Updater::upgrade_fs_image();
        if (res) {
            config_restore();
            res = Updater::upgrade_firmware();
            if (res) {
                pm.info("done! restart...");
            }
        }
        perform_upgrade = false;
    }

    if (broadcast_mqtt_settings_flag) {
        Broadcast::send_mqtt_settings();
        broadcast_mqtt_settings_flag = false;
    }

    if (perform_bus_scanning_flag) {
        BusScanner* bus;
        switch (perform_bus_scanning_bus) {
            case BS_I2C:
                bus = new I2CScanner();
                break;
            case BS_ONE_WIRE:
                bus = new DallasScanner();
                break;
            default:
                pm.error("uknown bus: " + String(perform_bus_scanning_bus, DEC));
                bus = nullptr;
        }
        if (bus) {
            bus->scan(configLiveJson);
        }
        perform_bus_scanning_flag = false;
    }

    if (perform_system_restart_flag) {
        ESP.restart();
    }
}

void setChipId() {
    chipId = getChipId();
    pm.info("id: " + chipId);
}

void setPreset(size_t num) {
    pm.info("preset #" + String(num, DEC));
    copyFile(getConfigFile(num, CT_CONFIG), DEVICE_CONFIG_FILE);
    copyFile(getConfigFile(num, CT_SCENARIO), DEVICE_SCENARIO_FILE);
    device_init();

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
    timeNow->setConfig(config.clock());

    ts.add(
        TIME_SYNC, 30000, [&](void*) {
            timeNow->hasSync();
        },
        nullptr, true);

    ts.add(
        TIME, 1000, [&](void*) {
            jsonWriteStr(configLiveJson, "time", timeNow->getTime());
            jsonWriteStr(configLiveJson, "timenow", timeNow->getTimeJson());
            fireEvent("timenow", "");
        },
        nullptr, true);
}

String scenarioBackup, configBackup, setupBackup;
void config_backup() {
    scenarioBackup = readFile(String(DEVICE_SCENARIO_FILE), 4096);
    configBackup = readFile(String(DEVICE_CONFIG_FILE), 4096);
    setupBackup = configSetupJson;
}

void config_restore() {
    writeFile(String(DEVICE_SCENARIO_FILE), scenarioBackup);
    writeFile(String(DEVICE_CONFIG_FILE), configBackup);
    writeFile("config.json", setupBackup);
    saveConfig();
}

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

    pm.info(String("Init"));
    init_mod();

    pm.info(String("Network"));
    startSTAMode();

    if (isNetworkActive()) {
        lastVersion = Updater::check();
        if (!lastVersion.isEmpty()) {
            pm.info("update: " + lastVersion);
        }
    };

    if (!TELEMETRY_UPDATE_INTERVAL) {
        pm.info("Telemetry: disabled");
    }
    telemetry_init();

    pm.info(String("HttpServer"));
    HttpServer::init();

    pm.info(String("WebAdmin"));
    web_init();

    pm.info("Broadcast");
    Broadcast::init();

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
