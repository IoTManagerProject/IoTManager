
#include <SSDP.h>
#include "Class/CallBackTest.h"
#include "Class/NotAsinc.h"
#include "Class/ScenarioClass.h"
#include "items/ButtonInClass.h"
#include "Cmd.h"
#include "Global.h"
#include "Init.h"
#include "ItemsList.h"
#include "Utils/Timings.h"
#include "Utils\WebUtils.h"

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

    myNotAsincActions = new NotAsinc(do_LAST);
    myScenario = new Scenario();

    pm.info("FS");
    fileSystemInit();

    pm.info("Config");
    loadConfig();

    pm.info("Clock");
    clock_init();

    pm.info("Commands");
    cmd_init();

    pm.info("Sensors");
    sensorsInit();

    pm.info("Init");
    all_init();

    pm.info("Network");
    startSTAMode();

    pm.info("Uptime");
    uptime_init();

    pm.info("telemetry");
    telemetry_init();

    pm.info("Updater");
    upgradeInit();

    pm.info("HttpServer");
    HttpServer::init();

    pm.info("WebAdmin");
    web_init();

#ifdef UDP_ENABLED
    pm.info("Broadcast UDP");
    udpInit();
#endif

    //pm.info("Ssdp Init");
    //SsdpInit();

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
#ifdef OTA_UPDATES_ENABLED
    ArduinoOTA.handle();
#endif
#ifdef WS_enable
    ws.cleanupClients();
#endif
#ifdef UDP_ENABLED
    loopUdp();
#endif
    timeNow->loop();
    MqttClient::loop();
    myButtonIn.loop();
    myScenario->loop();
    loopCmd();
    loopSerial();

    myNotAsincActions->loop();
    ts.update();
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
