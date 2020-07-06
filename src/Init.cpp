#include "Global.h"

#include "Messages.h"
#include "Broadcast.h"
#include "Logger.h"
#include "WebClient.h"
#include "Scenario.h"
#include "Sensors.h"
#include "MqttClient.h"
#include "NetworkManager.h"

static const char* MODULE = "Init";

void sensors_task() {
    ts.add(
        SENSORS, 1000, [&](void*) {
            Sensors::process();
        },
        nullptr, true);
}

void announce_task_init() {
    ts.add(
        ANNOUNCE, random(0.9 * config.general()->getBroadcastInterval() * ONE_SECOND_ms, config.general()->getBroadcastInterval()), [&](void*) {
            if (config.general()->isBroadcastEnabled()) {
                Messages::announce();
            } },
        nullptr, false);
}

void init_mod() {
    pm.info("Mqtt");
    MqttClient::setConfig(config.mqtt());

    pm.info("logger");
    Logger::init();

    pm.info("Commands");
    cmd_init();

    pm.info("Sensors");
    Sensors::init();
    sensors_task();

    device_init();

    timer_countdown_init();

    uptime_task_init();

    pm.info(String("Announce"));
    announce_task_init();

    ts.add(
        SYS_MEMORY, 10 * ONE_SECOND_ms, [&](void*) { print_sys_memory(); }, nullptr, false);

    ts.add(
        SYS_TIMINGS, ONE_MINUTE_ms, [&](void*) { print_sys_timins(); }, nullptr, false);

    if (NetworkManager::isNetworkActive()) {
        perform_updates_check_flag = true;
    };
}

void device_init() {
    pm.info("Start");
    clearWidgets();
    fileExecute(DEVICE_COMMAND_FILE);
    Scenario::init();
}

void uptime_task_init() {
    pm.info(String("uptime task"));
    ts.add(
        UPTIME, 5000, [&](void*) {
            runtime.write("uptime", timeNow->getUptime());
        },
        nullptr, true);
}

void config_init() {
    pm.info("config");
    load_config();

    runtime.load();
    runtime.write("chipID", getChipId());
    runtime.write("firmware_version", FIRMWARE_VERSION);

    prex = config.mqtt()->getPrefix() + "/" + getChipId();
}

void telemetry_init() {
    if (!TELEMETRY_UPDATE_INTERVAL) {
        pm.info("Telemetry: disabled");
        return;
    }
    if (TELEMETRY_UPDATE_INTERVAL) {
        ts.add(
            STATISTICS, TELEMETRY_UPDATE_INTERVAL, [&](void*) {
                if (!NetworkManager::isNetworkActive()) {
                    return;
                }
                String url = "http://backup.privet.lv/visitors/?";
                url += getChipId();
                url += "&";
#ifdef ESP8266
                url += "iot-manager_esp8266";
#endif
#ifdef ESP32
                url += "iot-manager_esp32";
#endif
                url += "&";
#ifdef ESP8266
                url += ESP.getResetReason();
#endif
#ifdef ESP32
                url += "Power on";
#endif
                url += "&";
                url += FIRMWARE_VERSION;
                String stat = WebClient::get(url);
            },
            nullptr, true);
    }
}