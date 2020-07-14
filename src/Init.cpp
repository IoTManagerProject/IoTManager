#include "Global.h"

#include "Messages.h"
#include "Broadcast.h"
#include "Logger.h"
#include "WebClient.h"
#include "Scenario.h"
#include "Sensors.h"
#include "Timers.h"
#include "MqttClient.h"
#include "NetworkManager.h"

static const char* MODULE = "Init";

void sensors_task() {
    ts.add(
        SENSORS, 1000, [&](void*) {
            Sensors::update();
        },
        nullptr, true);
}

void announce_task() {
    ts.add(
        ANNOUNCE, random(0.9 * config.general()->getBroadcastInterval() * ONE_SECOND_ms, config.general()->getBroadcastInterval()), [&](void*) {
            if (config.general()->isBroadcastEnabled()) {
                String data;
                data += getChipId();
                data += ";";
                data += config.general()->getBroadcastName();
                data += ";";
                data += NetworkManager::getHostIP().toString();
                data += ";";

                Messages::post(BM_ANNOUNCE, data);
            }
        },
        nullptr, false);
}

void timers_task() {
    ts.add(
        TIMERS, ONE_SECOND_ms, [&](void*) { Timers::loop(); }, nullptr, true);
}

void uptime_task() {
    ts.add(
        UPTIME, 5 * ONE_SECOND_ms, [&](void*) { runtime.write("uptime", now.getUptime()); }, nullptr, true);
}

void init_mod() {
    MqttClient::init();

    Logger::init();

    cmd_init();

    sensors_task();

    timers_task();

    uptime_task();

    announce_task();

    ts.add(
        SYS_MEMORY, 10 * ONE_SECOND_ms, [&](void*) { print_sys_memory(); }, nullptr, false);

    ts.add(
        SYS_TIMINGS, ONE_MINUTE_ms, [&](void*) { print_sys_timins(); }, nullptr, false);

    device_init();

    if (NetworkManager::isNetworkActive()) {
        perform_updates_check();
    };
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
                WebClient::get(url);
            },
            nullptr, true);
    }
}