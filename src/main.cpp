#include "Global.h"

#include "Events.h"
#include "Broadcast.h"
#include "MqttClient.h"
#include "HttpServer.h"
#include "Bus/I2CScanner.h"
#include "Bus/DallasScanner.h"
#include "TickerScheduler/Metric.h"
#include "Sensors.h"
#include "Logger.h"

static const char* MODULE = "Main";

void flag_actions();
void config_backup();
void config_restore();

enum LoopItems {
    LI_CLOCK,
    LT_FLAG_ACTION,
    LI_MQTT_CLIENT,
    LI_CMD,
    LI_BUTTON,
    LI_SCENARIO,
    LI_BROADCAST,
    LI_SERIAL,
    LT_TASKS,
    LT_LOGGER,
    NUM_LOOP_ITEMS
};

Metric m;
boolean initialized = false;

boolean perform_updates_check_flag = false;
void perform_updates_check() {
    pm.info("updates check");
    perform_updates_check_flag = true;
}

boolean perform_upgrade_flag = false;
void perform_upgrade() {
    pm.info("upgrade");
    perform_upgrade_flag = true;
}

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

boolean perform_logger_clear_flag = false;
void perform_logger_clear() {
    pm.info("logger clear");
    perform_logger_clear_flag = true;
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
    flag_actions();
    m.add(LT_FLAG_ACTION);

    MqttClient::loop();
    m.add(LI_MQTT_CLIENT);

    loop_cmd();
    m.add(LI_CMD);

    loop_button();
    m.add(LI_BUTTON);

    if (config.general()->isScenarioEnabled()) {
        Scenario::process(&events);
        m.add(LI_SCENARIO);
    }

    if (config.general()->isBroadcastEnabled()) {
        Broadcast::loop();
        m.add(LI_BROADCAST);
    }

    loop_serial();

    m.add(LI_SERIAL);

    ts.update();
    m.add(LT_TASKS);

    Logger::loop();
    m.add(LT_LOGGER);
}

void flag_actions() {
    if (config.hasChanged()) {
        pm.info("store " DEVICE_CONFIG_FILE);
        save_config();
    }

    if (mqtt_restart_flag) {
        MqttClient::setConfig(config.mqtt());
        MqttClient::reconnect();
        mqtt_restart_flag = false;
    }

    if (perform_updates_check_flag) {
        lastVersion = Updater::check();
        runtime.write("last_version", lastVersion);
        perform_updates_check_flag = false;
    }

    if (perform_upgrade_flag) {
        config_backup();
        bool res = Updater::upgrade_fs_image();
        if (res) {
            config_restore();
            res = Updater::upgrade_firmware();
            if (res) {
                pm.info("done! restart...");
            }
        } else {
            pm.error("upgrade image");
        }
        perform_upgrade_flag = false;
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
            String buf;
            bus->scan(buf);
            liveData.write(bus->getTag(), buf);
        }
        perform_bus_scanning_flag = false;
    }

    if (perform_logger_clear_flag) {
        Logger::clear();
        perform_logger_clear_flag = false;
    }

    if (perform_system_restart_flag) {
        ESP.restart();
    }
}

void setPreset(size_t num) {
    pm.info("preset #" + String(num, DEC));
    copyFile(getConfigFile(num, CT_CONFIG), DEVICE_COMMAND_FILE);
    copyFile(getConfigFile(num, CT_SCENARIO), DEVICE_SCENARIO_FILE);
    device_init();
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
            runtime.write("time", timeNow->getTime());
            runtime.write("timenow", timeNow->getTimeJson());
            Events::fire("timenow");
        },
        nullptr, true);
}

String scenarioBackup, commandBackup, configBackup;
void config_backup() {
    readFile(DEVICE_SCENARIO_FILE, scenarioBackup);
    readFile(DEVICE_COMMAND_FILE, commandBackup);
    readFile(DEVICE_CONFIG_FILE, configBackup);
}

void config_restore() {
    writeFile(DEVICE_SCENARIO_FILE, scenarioBackup);
    writeFile(DEVICE_COMMAND_FILE, commandBackup);
    writeFile(DEVICE_CONFIG_FILE, configBackup);
    load_config();
}

void setup() {
    WiFi.setAutoConnect(false);
    WiFi.persistent(false);

    Serial.begin(115200);
    Serial.flush();
    Serial.println();
    Serial.println("--------------started----------------");

    pm.info(String("FS"));
    fileSystemInit();
    config_init();

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

    // ts.add(
    // SYS_STAT, 1000 * 60, [&](void*) { print_sys_stat }, nullptr, false);

    just_load = false;
    initialized = true;
}

void print_sys_stat() {
    pm.info(getMemoryStatus());
    m.print(Serial);
    ts.printMetric(Serial);
    m.reset();
    ts.resetMetric();
}
