
#include <SSDP.h>

#include "BufferExecute.h"
#include "Bus.h"
#include "Class/CallBackTest.h"
#include "Class/NotAsync.h"
#include "Class/ScenarioClass.h"
#include "Cmd.h"
#include "Global.h"
#include "Init.h"
#include "ItemsList.h"
#include "RemoteOrdersUdp.h"
#include "Utils/StatUtils.h"
#include "Utils/Timings.h"
#include "Utils/WebUtils.h"
#include "items/ButtonInClass.h"
#include "items/LoggingClass.h"
#include "items/ImpulsOutClass.h"
#include "items/SensorDallas.h"
#include "Telegram.h"

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

    myNotAsyncActions = new NotAsync(do_LAST);
    myScenario = new Scenario();

    fileSystemInit();
    SerialPrint("I", "FS", "FS Init");

    loadConfig();
    SerialPrint("I", "Conf", "Config Init");

    clock_init();
    SerialPrint("I", "Time", "Clock Init");

    handle_time_init();
    SerialPrint("I", "Time", "Handle time init(");

    sensorsInit();
    SerialPrint("I", "Sensors", "Sensors Init");

    itemsListInit();
    SerialPrint("I", "Items", "Items Init");

    all_init();
    SerialPrint("I", "Init", "Init Init");

    routerConnect();
    SerialPrint("I", "WIFI", "Network Init");

    telegramInit();
    SerialPrint("I", "Telegram", "Telegram Init");

    uptime_init();
    SerialPrint("I", "Uptime", "Uptime Init");

    upgradeInit();
    SerialPrint("I", "Update", "Updater Init");

    HttpServer::init();
    SerialPrint("I", "HTTP", "HttpServer Init");

    web_init();
    SerialPrint("I", "Web", "WebAdmin Init");

    initSt();
    SerialPrint("I", "Stat", "Stat Init");

#ifdef UDP_ENABLED
    SerialPrint("I", "UDP", "Udp Init");
    asyncUdpInit();
#endif

    SerialPrint("I", "Bus", "Bus Init");
    busInit();

#ifdef SSDP_ENABLED
    SerialPrint("I", "SSDP", "Ssdp Init");
    SsdpInit();
#endif



    //esp_log_level_set("esp_littlefs", ESP_LOG_NONE);

    ts.add(
        TEST, 1000 * 60, [&](void*) {
            SerialPrint("I", "System", printMemoryStatus());
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
    timeNow->loop();
    mqttLoop();
    myButtonIn.loop();
    myScenario->loop();
    loopCmdExecute();
    //loopSerial();

    myNotAsyncActions->loop();
    ts.update();

    handleTelegram();

    if (myLogging != nullptr) {
        for (unsigned int i = 0; i < myLogging->size(); i++) {
            myLogging->at(i).loop();
        }
    }

    if (myImpulsOut != nullptr) {
        for (unsigned int i = 0; i < myImpulsOut->size(); i++) {
            myImpulsOut->at(i).loop();
        }
    }

    if (mySensorDallas2 != nullptr) {
        for (unsigned int i = 0; i < mySensorDallas2->size(); i++) {
            mySensorDallas2->at(i).loop();
        }
    }
}