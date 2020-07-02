#include "Global.h"

#include "Runtime.h"
#include "WebClient.h"
#include "Sensors.h"
#include "MqttClient.h"

static const char* MODULE = "Init";

void init_mod() {
    pm.info("Mqtt");
    MqttClient::setConfig(config.mqtt());

    pm.info("Commands");
    cmd_init();

    device_init();

    pm.info(String("Scenario"));
    Scenario::init();

    pm.info(String("Sensors"));
    Sensors::init();

    timer_countdown_init();

    uptime_task_init();
}

void device_init() {
    for (int i = LOG1; i <= LOG5; i++) {
        ts.remove(i);
    }
    clearWidgets();
    fileExecute(DEVICE_COMMAND_FILE);
    Scenario::reinit();
}

void uptime_task_init() {
    pm.info(String("uptime task"));
    ts.add(
        UPTIME, 5000, [&](void*) {
            Runtime::write("uptime", timeNow->getUptime());
        },
        nullptr, true);
}

void config_init() {
    pm.info("config");
    load_config();
    
    Runtime::load();
    Runtime::write("chipID", getChipId());
    Runtime::write("firmware_version", FIRMWARE_VERSION);

    prex = config.mqtt()->getPrefix() + "/" + getChipId();
}

void telemetry_init() {
    if (TELEMETRY_UPDATE_INTERVAL) {
        ts.add(
            STATISTICS, TELEMETRY_UPDATE_INTERVAL, [&](void*) {
                if (isNetworkActive()) {
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
                    url += String(FIRMWARE_VERSION);
                    String stat = WebClient::get(url);
                }
            },
            nullptr, true);
    }
}