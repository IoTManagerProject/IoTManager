#include "Global.h"

#include "HttpServer.h"
#include "Bus/BusScanner.h"
#include "Utils/Timings.h"

void not_async_actions();

static const char* MODULE = "Main";

void do_fscheck(String& results) {
}

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

    pm.info("Clock");
    rtc = new Clock();
    rtc->setNtpPool(jsonReadStr(configSetupJson, "ntp"));
    rtc->setTimezone(jsonReadStr(configSetupJson, "timezone").toInt());

    ts.add(
        TIME_SYNC, 30000, [&](void*) {
            rtc->hasSync();
        },
        nullptr, true);

#ifdef UDP_ENABLED
    UDP_init();
    pm.info("Broadcast");
#endif
    ts.add(
        TEST, 10000, [&](void*) {
            pm.info(printMemoryStatus());
        },
        nullptr, true);

    just_load = false;
}

void saveConfig() {
    writeFile(String("config.json"), configSetupJson);
}

Timings metric;

void loop() {
#ifdef OTA_UPDATES_ENABLED
    ArduinoOTA.handle();
#endif
#ifdef WS_enable
    ws.cleanupClients();
#endif
    metric.add(MT_ONE);
    not_async_actions();

    metric.add(MT_TWO);
    MqttClient::loop();

    loopCmd();

    loopButton();

    loopScenario();

#ifdef UDP_ENABLED
    loopUdp();
#endif

    loopSerial();

    ts.update();

    if (metric._counter > 100000) {
        metric.print();
    } else {
        metric.count();
    }
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

    if (busScanFlag) {
        String res = "";
        BusScanner* scanner = BusScannerFactory::get(res, busToScan);
        scanner->scan();
        jsonWriteStr(configLiveJson, BusScannerFactory::label(busToScan), res);
        busScanFlag = false;
    }

    if (fsCheckFlag) {
        String buf;
        do_fscheck(buf);
        jsonWriteStr(configLiveJson, "fscheck", buf);
        fsCheckFlag = false;
    }
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
