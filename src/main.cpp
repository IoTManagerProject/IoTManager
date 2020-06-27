#include "Global.h"

#include "HttpServer.h"
#include "Bus/BusScanner.h"
#include "Utils/Timings.h"

void not_async_actions();

static const char* MODULE = "Main";

Timings metric;
boolean initialized = false;

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

    pm.info("Clock");
    clock_init();

    pm.info("Commands");
    cmd_init();

    pm.info("Sensors");
    sensors_init();

    pm.info("Init");
    all_init();

    pm.info("Network");
    startSTAMode();

    pm.info("Uptime");
    uptime_init();

    if (!TELEMETRY_UPDATE_INTERVAL) {
        pm.info("Telemetry: Disabled");
    }
    telemetry_init();

    pm.info("Updater");
    initUpdater();

    pm.info("HttpServer");
    HttpServer::init();

    pm.info("WebAdmin");
    web_init();

#ifdef UDP_ENABLED
    pm.info("Broadcast UDP");
    UDP_init();
#endif
    ts.add(
        TEST, 1000 * 60, [&](void*) {
            pm.info(printMemoryStatus());
        },
        nullptr, true);

    just_load = false;

    initialized = true;
}

void loop() {
    if (!initialized) {
        return;
    }
    timeNow->loop();

#ifdef OTA_UPDATES_ENABLED
    ArduinoOTA.handle();
#endif
#ifdef WS_enable
    ws.cleanupClients();
#endif
    not_async_actions();

    MqttClient::loop();

    loopCmd();

    loopButton();

    loopScenario();

#ifdef UDP_ENABLED
    loopUdp();
#endif

    loopSerial();

    ts.update();
}

void not_async_actions() {
    if (mqttParamsChanged) {
        MqttClient::reconnect();
        mqttParamsChanged = false;
    }

    getLastVersion();

    flashUpgrade();

#ifdef UDP_ENABLED
    do_udp_data_parse();
    do_mqtt_send_settings_to_udp();
#endif

    do_scan_bus();

    do_check_fs();
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
    String fileName;
    fileName.toLowerCase();
    fileName = deleteBeforeDelimiter(fileName, " ");
    fileName.replace(" ", ".");
    fileName.replace("..", ".");
    fileName = Folder + "/" + fileName + ".txt";

    jsonWriteStr(configLiveJson, "test", fileName);
}

void sendConfig(String topic, String widgetConfig, String key, String date) {
    yield();
    topic = jsonReadStr(configSetupJson, "mqttPrefix") + "/" + chipId + "/" + topic + "/status";
    String outer = "{\"widgetConfig\":";
    String inner = "{\"";
    inner = inner + key;
    inner = inner + "\":\"";
    inner = inner + date;
    inner = inner + "\"";
    inner = inner + "}}";
    String t = outer + inner;
    yield();
}

void setChipId() {
    chipId = getChipId();
    Serial.println(chipId);
}

void saveConfig() {
    writeFile(String("config.json"), configSetupJson);
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

void do_fscheck(String& results) {
    // TODO Проверка наличие важных файлов, возможно версии ФС
}

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
        String res = "";
        BusScanner* scanner = BusScannerFactory::get(res, busToScan);
        scanner->scan();
        jsonWriteStr(configLiveJson, BusScannerFactory::label(busToScan), res);
        busScanFlag = false;
    }
}

void do_check_fs() {
    if (fsCheckFlag) {
        String buf;
        do_fscheck(buf);
        jsonWriteStr(configLiveJson, "fscheck", buf);
        fsCheckFlag = false;
    }
}
