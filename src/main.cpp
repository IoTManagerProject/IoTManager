
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

    SerialPrint("I","FS","FS");
    fileSystemInit();

    SerialPrint("I","Conf","Config");
    loadConfig();

    SerialPrint("I","Time","Clock");
    clock_init();

    SerialPrint("I","CMD","Commands");
    cmd_init();

    SerialPrint("I","Sensors","Sensors");
    sensorsInit();

    SerialPrint("I","Init","Init");
    all_init();

    SerialPrint("I","WIFI","Network");
    startSTAMode();

    SerialPrint("I","Uptime","Uptime");
    uptime_init();

    SerialPrint("I","Update","Updater");
    upgradeInit();

    SerialPrint("I","HTTP","HttpServer");
    HttpServer::init();

    SerialPrint("I","Web","WebAdmin");
    web_init();

    SerialPrint("I","Stat","InitSt");
    initSt();

    SerialPrint("I","UDP","asyncUdpInit");
    asyncUdpInit();

#ifdef UDP_ENABLED
    SerialPrint("I","module","Broadcast UDP");
    udpInit();
#endif
#ifdef SSDP_EN
    SerialPrint("I","SSDP","Ssdp Init");
    SsdpInit();
#endif

    ts.add(
        TEST, 1000 * 60, [&](void*) {
            SerialPrint("I","System",printMemoryStatus());
            
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
