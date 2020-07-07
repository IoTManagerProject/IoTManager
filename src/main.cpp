#include "Global.h"

#include "Scenario.h"
#include "NetworkManager.h"
#include "Events.h"
#include "Messages.h"
#include "Broadcast.h"
#include "Devices.h"
#include "MqttClient.h"
#include "HttpServer.h"
#include "Sensors/I2CScanner.h"
#include "Sensors/OneWireScanner.h"
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
    LI_ITEMS,
    LI_SCENARIO,
    LI_BROADCAST,
    LI_SERIAL,
    LT_TASKS,
    LT_LOGGER,
    NUM_LOOP_ITEMS
};

Metric m;
boolean initialized = false;
BusScanner* bus = NULL;

bool perform_mqtt_restart_flag = false;
void perform_mqtt_restart() {
    pm.info("mqtt restart");
    perform_mqtt_restart_flag = true;
}

bool perform_updates_check_flag = false;
void perform_updates_check() {
    pm.info("updates check");
    perform_updates_check_flag = true;
}

bool perform_upgrade_flag = false;
void perform_upgrade() {
    pm.info("upgrade");
    perform_upgrade_flag = true;
}

bool broadcast_mqtt_settings_flag = false;
void broadcast_mqtt_settings() {
    broadcast_mqtt_settings_flag = true;
}

boolean perform_bus_scanning_flag = false;
BusScanner_t perform_bus_scanning_bus;
void perform_bus_scanning(BusScanner_t bus) {
    pm.info("scan bus");
    perform_bus_scanning_flag = true;
    perform_bus_scanning_bus = bus;
}

bool perform_system_restart_flag = false;
void perform_system_restart() {
    pm.info("system restart");
    perform_system_restart_flag = true;
}

bool perform_logger_clear_flag = false;
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

    ArduinoOTA.handle();

    ws.cleanupClients();

    flag_actions();
    m.add(LT_FLAG_ACTION);

    MqttClient::loop();
    m.add(LI_MQTT_CLIENT);

    loop_cmd();
    m.add(LI_CMD);

    loop_items();
    m.add(LI_ITEMS);

    if (config.general()->isScenarioEnabled()) {
        Scenario::process(Events::get());
        m.add(LI_SCENARIO);
    }

    if (config.general()->isBroadcastEnabled()) {
        Messages::loop();
        Broadcast::loop();
        m.add(LI_BROADCAST);
    }

    ts.update();
    m.add(LT_TASKS);

    Logger::loop();
    m.add(LT_LOGGER);

    if (config.hasChanged()) {
        save_config();
    }
}

void flag_actions() {
    if (perform_mqtt_restart_flag) {
        MqttClient::reconnect();
        perform_mqtt_restart_flag = false;
    }

    if (perform_updates_check_flag) {
        String res = Updater::check();
        runtime.write("last_version", res);
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
        DynamicJsonBuffer jsonBuffer;
        JsonObject& root = jsonBuffer.createObject();
        config.mqtt()->save(root);

        String buf;
        root.printTo(buf);
        Broadcast::send(HEADER_MQTT_SETTINGS, buf);

        broadcast_mqtt_settings_flag = false;
    }

    if (perform_bus_scanning_flag) {
        if (bus == NULL) {
            switch (perform_bus_scanning_bus) {
                case BS_I2C:
                    bus = new I2CScanner();
                    break;
                case BS_ONE_WIRE:
                    bus = new OneWireScanner();
                    break;
                default:
                    pm.error("uknown bus: " + String(perform_bus_scanning_bus, DEC));
            }
        }
        if (bus) {
            if (bus->scan()) {
                pm.info("scan done");
                runtime.write(bus->tag(), bus->results());
                perform_bus_scanning_flag = false;
                delete bus;
                bus = NULL;
            }
        }
    }

    if (perform_logger_clear_flag) {
        Logger::clear();
        perform_logger_clear_flag = false;
    }

    if (perform_system_restart_flag) {
        ESP.restart();
    }
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
    pm.info("init");
    timeNow = new Clock();
    timeNow->setConfig(config.clock());

    ts.add(
        TIME_SYNC, 30000, [&](void*) {
            timeNow->sync();
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

void print_sys_memory() {
    pm.info(getHeapStats());
}

void print_sys_timins() {
    m.print(Serial);
    m.reset();

    ts.print(Serial);
    ts.reset();
}

void setup() {
    WiFi.setAutoConnect(false);
    WiFi.persistent(false);

    Serial.begin(115200);
    Serial.flush();
    Serial.println();
    Serial.println("--------------started----------------");

    fileSystemInit();

    config_init();

    clock_init();

    NetworkManager::init();

    pm.info("HttpServer");
    HttpServer::init();

    pm.info("WebAdmin");
    web_init();

    pm.info("Broadcast");
    Broadcast::init();

    telemetry_init();

    pm.info("Devices");
    Devices::init();

    pm.info("Init");
    init_mod();

    initialized = true;
}
