#include "Global.h"

void not_async_actions();

static const char* MODULE = "Main";

void setup() {
    WiFi.setAutoConnect(false);
    WiFi.persistent(false);

    Serial.begin(115200);
    Serial.flush();
    Serial.println();
    Serial.println("--------------started----------------");

    setChipId();

    pm.info("FS");
    fileSystemInit();

    pm.info("Config");
    loadConfig();

    pm.info("Commands");
    CMD_init();

    pm.info("Sensors");
    sensors_init();

    pm.info("Init");
    All_init();

    pm.info("Network");
    startSTAMode();

    pm.info("Uptime");
    uptime_init();

    pm.info("Telemery: " + !TELEMETRY_UPDATE_INTERVAL ? "Disabled" : "Enabled");
    telemetry_init();

    pm.info("Updater");
    init_updater();

    pm.info("WebServer");
    Web_server_init();

    pm.info("WebAdmin");
    web_init();

    pm.info("TimeSync");
    ts.add(
        TIME_SYNC, 30000, [&](void*) {
            startTimeSync();
        },
        nullptr, true);

#ifdef UDP_ENABLED
    UDP_init();
    pm.info("Broadcast");
#endif
    ts.add(
        TEST, 10000, [&](void*) {
            printMemoryStatus();
        },
        nullptr, true);

    just_load = false;
}

void saveConfig() {
    writeFile("config.json", configSetupJson);
}

void loop() {
#ifdef OTA_UPDATES_ENABLED
    ArduinoOTA.handle();
#endif

#ifdef WS_enable
    ws.cleanupClients();
#endif

    not_async_actions();

    handleMQTT();
    handleCMD_loop();
    handleButton();
    handleScenario();
#ifdef UDP_ENABLED
    handleUdp();
#endif
    ts.update();
}

void not_async_actions() {
    do_mqtt_connection();
    do_upgrade_url();
    do_upgrade();

#ifdef UDP_ENABLED
    do_udp_data_parse();
    do_mqtt_send_settings_to_udp();
#endif

    do_i2c_scanning();
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

void safeDataToFile(String data, String Folder) {
    //String fileName = GetDate();
    String fileName;
    fileName.toLowerCase();
    fileName = deleteBeforeDelimiter(fileName, " ");
    fileName.replace(" ", ".");
    fileName.replace("..", ".");
    fileName = Folder + "/" + fileName + ".txt";
    // addFile(fileName, GetTime() + "/" + data);
    Serial.println(fileName);
    jsonWriteStr(configLiveJson, "test", fileName);
}

void sendCONFIG(String topik, String widgetConfig, String key, String date) {
    yield();
    topik = jsonReadStr(configSetupJson, "mqttPrefix") + "/" + chipId + "/" + topik + "/status";
    String outer = "{\"widgetConfig\":";
    String inner = "{\"";
    inner = inner + key;
    inner = inner + "\":\"";
    inner = inner + date;
    inner = inner + "\"";
    inner = inner + "}}";
    String t = outer + inner;
    //Serial.println(t);
    //client_mqtt.publish(MQTT::Publish(topik, t).set_qos(1));
    yield();
}

void setChipId() {
    chipId = getChipId();
    Serial.println(chipId);
}

#ifdef ESP8266
#ifdef LED_PIN

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
#endif
#endif
