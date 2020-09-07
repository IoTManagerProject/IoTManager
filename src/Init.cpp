#include "Global.h"

void handle_uptime();
void handle_statistics();
void telemetry_init();

void loadConfig() {
    configSetupJson = readFile("config.json", 4096);
    configSetupJson.replace(" ", "");
    configSetupJson.replace("\r\n", "");

    jsonWriteStr(configSetupJson, "chipID", chipId);
    jsonWriteStr(configSetupJson, "firmware_version", FIRMWARE_VERSION);

    prex = jsonReadStr(configSetupJson, "mqttPrefix") + "/" + chipId;

    Serial.println(configSetupJson);
}

void all_init() {
    Device_init();
    loadScenario();
    Timer_countdown_init();
}

void Device_init() {
    logging_value_names_list = "";
    enter_to_logging_counter = LOG1 - 1;

    analog_value_names_list = "";
    enter_to_analog_counter = 0;

    dallas_value_name = "";
    enter_to_dallas_counter = 0;

    levelPr_value_name = "";
    ultrasonicCm_value_name = "";

    dhtT_value_name = "";
    dhtH_value_name = "";

    bmp280T_value_name = "";
    bmp280P_value_name = "";

    bme280T_value_name = "";
    bme280P_value_name = "";
    bme280H_value_name = "";
    bme280A_value_name = "";

    int array_sz = sizeof(sensors_reading_map) / sizeof(sensors_reading_map[0]);

    for (int i = 0; i < array_sz; i++) {
        sensors_reading_map[i] = 0;
    }

    for (int i = LOG1; i <= LOG5; i++) {
        ts.remove(i);
    }

#ifdef LAYOUT_IN_RAM
    all_widgets = "";
#else
    removeFile(String("layout.txt"));
#endif

    fileExecute(String(DEVICE_CONFIG_FILE));
    //outcoming_date();
}
//-------------------------------сценарии-----------------------------------------------------

void loadScenario() {
    if (jsonReadStr(configSetupJson, "scen") == "1") {
        scenario = readFile(String(DEVICE_SCENARIO_FILE), 2048);
    }
}

void uptime_init() {
    ts.add(
        UPTIME, 5000, [&](void*) {
            handle_uptime();
        },
        nullptr, true);
}


void handle_uptime() {
    jsonWriteStr(configSetupJson, "uptime", timeNow->getUptime());
}

void handle_statistics() {
    if (isNetworkActive()) {
        String urls = "http://backup.privet.lv/visitors/?";
        //-----------------------------------------------------------------
        urls += WiFi.macAddress().c_str();
        urls += "&";
        //-----------------------------------------------------------------
#ifdef ESP8266
        urls += "iot-manager_esp8266";
#endif
#ifdef ESP32
        urls += "iot-manager_esp32";
#endif
        urls += "&";
#ifdef ESP8266
        urls += ESP.getResetReason();
#endif
#ifdef ESP32
        urls += "Power on";
#endif
        urls += "&";
        urls += String(FIRMWARE_VERSION);
        String stat = getURL(urls);
    }
}