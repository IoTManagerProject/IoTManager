
#include <SSDP.h>

#include "BufferExecute.h"
#include "Class/CallBackTest.h"
#include "Class/NotAsinc.h"
#include "Class/ScenarioClass.h"
#include "Cmd.h"
#include "Global.h"
#include "Init.h"
#include "ItemsList.h"
#include "Utils/StatUtils.h"
#include "Utils/Timings.h"
#include "Utils\WebUtils.h"
#include "items/ButtonInClass.h"
//#include "RemoteOrdersUdp.h"
#include "Bus.h"

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

    SerialPrint("I", "FS", "FS Init");
    fileSystemInit();

    SerialPrint("I", "Conf", "Config Init");
    loadConfig();

    SerialPrint("I", "Time", "Clock Init");
    clock_init();

    SerialPrint("I", "CMD", "Commands Init");
    cmd_init();

    SerialPrint("I", "Sensors", "Sensors Init");
    sensorsInit();

    SerialPrint("I", "Items", "Items Init");
    itemsListInit();

    SerialPrint("I", "Init", "Init Init");
    all_init();

    SerialPrint("I", "WIFI", "Network Init");
    routerConnect();

    SerialPrint("I", "Uptime", "Uptime Init");
    uptime_init();

    SerialPrint("I", "Update", "Updater Init");
    upgradeInit();

    SerialPrint("I", "HTTP", "HttpServer Init");
    HttpServer::init();

    SerialPrint("I", "Web", "WebAdmin Init");
    web_init();

    SerialPrint("I", "Stat", "Stat Init");
    initSt();

    //SerialPrint("I","UDP","Udp Init");
    //asyncUdpInit();

    SerialPrint("I", "Bus", "Bus Init");
    busInit();

#ifdef SSDP_EN
    SerialPrint("I", "SSDP", "Ssdp Init");
    SsdpInit();
#endif

    ts.add(
        TEST, 1000 * 60, [&](void*) {
            SerialPrint("I", "System", printMemoryStatus());
        },
        nullptr, true);

    just_load = false;
    initialized = true;  //this second POST makes the data to be processed (you don't need to connect as "keep-alive" for that to work)
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
    timeNow->loop();
    mqttLoop();
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
