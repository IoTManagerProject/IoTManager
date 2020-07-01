#include "Global.h"

#include "WebClient.h"
#include "Sensors.h"
#include "MqttClient.h"

void handle_uptime();
void handle_statistics();
void telemetry_init();

static const char* MODULE = "Init";

void loadConfig() {
    configSetupJson = readFile("config.json", 4096);

    config.load(configSetupJson);

    String tmp = "{}";
    config.save(tmp);
    pm.info(tmp);

    configSetupJson.replace(" ", "");
    configSetupJson.replace("\r\n", "");

    jsonWriteStr(configSetupJson, "chipID", chipId);
    jsonWriteStr(configSetupJson, "firmware_version", FIRMWARE_VERSION);

    prex = jsonReadStr(configSetupJson, "mqttPrefix") + "/" + chipId;

    Serial.println(configSetupJson);
}

void init_mod() {
    pm.info("Mqtt");
    MqttClient::setConfig(config.mqtt());

    pm.info(String("Commands"));
    cmd_init();

    device_init();

    pm.info(String("Scenario"));
    Scenario::init();

    pm.info(String("Sensors"));
    Sensors::init();

    Timer_countdown_init();

    pm.info(String("Uptime"));
    uptime_init();
}

void device_init() {
    for (int i = LOG1; i <= LOG5; i++) {
        ts.remove(i);
    }
    clearWidgets();
    fileExecute(DEVICE_CONFIG_FILE);
    Scenario::reinit();
}

void uptime_init() {
    ts.add(
        UPTIME, 5000, [&](void*) {
            handle_uptime();
        },
        nullptr, true);
}

void telemetry_init() {
    if (TELEMETRY_UPDATE_INTERVAL) {
        ts.add(
            STATISTICS, TELEMETRY_UPDATE_INTERVAL, [&](void*) {
                handle_statistics();
            },
            nullptr, true);
    }
}

void handle_uptime() {
    jsonWriteStr(configSetupJson, "uptime", timeNow->getUptime());
}

void handle_statistics() {
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
}