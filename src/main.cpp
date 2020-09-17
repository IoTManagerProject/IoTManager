
#include <SSDP.h>

#include "BufferExecute.h"
#include "Class/CallBackTest.h"
#include "Class/NotAsinc.h"
#include "Class/ScenarioClass.h"
#include "Utils/StatUtils.h"
#include "Cmd.h"
#include "Global.h"
#include "Init.h"
#include "ItemsList.h"
#include "Utils/Timings.h"
#include "Utils\WebUtils.h"
#include "items/ButtonInClass.h"
#include "RemoteOrdersUdp.h"

void not_async_actions();


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

    SerialPrint("I","module","FS");
    fileSystemInit();

    SerialPrint("I","module","Config");
    loadConfig();

    SerialPrint("I","module","Clock");
    clock_init();

    SerialPrint("I","module","Commands");
    cmd_init();

    SerialPrint("I","module","Sensors");
    sensorsInit();

    SerialPrint("I","module","Init");
    all_init();

    SerialPrint("I","module","Network");
    startSTAMode();

    SerialPrint("I","module","Uptime");
    uptime_init();

    SerialPrint("I","module","Updater");
    upgradeInit();

    SerialPrint("I","module","HttpServer");
    HttpServer::init();

    SerialPrint("I","module","WebAdmin");
    web_init();

    SerialPrint("I","module","InitSt");
    initSt();

    SerialPrint("I","module","asyncUdpInit");
    asyncUdpInit();

#ifdef UDP_ENABLED
    SerialPrint("I","module","Broadcast UDP");
    udpInit();
#endif
#ifdef SSDP_EN
    SerialPrint("I","module","Ssdp Init");
    SsdpInit();
#endif

    ts.add(
        TEST, 1000 * 60, [&](void*) {
            SerialPrint("I","module",printMemoryStatus());
            
        },
        nullptr, true);

    just_load = false;
    initialized = true;                                                                                                    //this second POST makes the data to be processed (you don't need to connect as "keep-alive" for that to work)
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
    loopCmdExecute();
    //loopSerial();

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
