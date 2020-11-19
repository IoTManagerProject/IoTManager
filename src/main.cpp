
#include <SSDP.h>

#include "BufferExecute.h"
#include "Bus.h"
#include "Class/CallBackTest.h"
#include "Class/NotAsync.h"
#include "Class/ScenarioClass3.h"
#include "Cmd.h"
#include "Global.h"
#include "Init.h"
#include "ItemsList.h"
#include "RemoteOrdersUdp.h"
#include "Utils/StatUtils.h"
#include "Utils/Timings.h"
#include "Utils/WebUtils.h"
#include "items/ButtonInClass.h"
#include "items/vLogging.h"
#include "items/vImpulsOut.h"
#include "items/vSensorDallas.h"
#include "items/vCountDown.h"
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
    Serial.println(F("--------------started----------------"));

    setChipId();

    myNotAsyncActions = new NotAsync(do_LAST);
    myScenario = new Scenario();

    fileSystemInit();
    SerialPrint("I", F("FS"), F("FS Init"));

    loadConfig();
    SerialPrint("I", F("Conf"), F("Config Init"));

    clock_init();
    SerialPrint("I", F("Time"), F("Clock Init"));

    handle_time_init();
    SerialPrint("I", F("Time"), F("Handle time init"));

    sensorsInit();
    SerialPrint("I", F("Sensors"), F("Sensors Init"));

    itemsListInit();
    SerialPrint("I", F("Items"), F("Items Init"));

    all_init();
    SerialPrint("I", F("Init"), F("Init Init"));

    routerConnect();
    SerialPrint("I", F("WIFI"), F("Network Init"));

    telegramInit();
    SerialPrint("I", F("Telegram"), F("Telegram Init"));

    uptime_init();
    SerialPrint("I", F("Uptime"), F("Uptime Init"));

    upgradeInit();
    SerialPrint("I", F("Update"), F("Updater Init"));

    HttpServer::init();
    SerialPrint("I", F("HTTP"), F("HttpServer Init"));

    web_init();
    SerialPrint("I", F("Web"), F("WebAdmin Init"));

    initSt();
    SerialPrint("I", F("Stat"), F("Stat Init"));

#ifdef UDP_ENABLED
    SerialPrint("I", F("UDP"), "Udp Init");
    asyncUdpInit();
#endif

    SerialPrint("I", F("Bus"), F("Bus Init"));
    busInit();

#ifdef SSDP_ENABLED
    SerialPrint("I", F("SSDP"), F("Ssdp Init"));
    SsdpInit();
#endif

    //esp_log_level_set("esp_littlefs", ESP_LOG_NONE);

    ts.add(
        SYGNAL, 1000 * 60, [&](void*) {
            SerialPrint("I", "System", printMemoryStatus());
            switch (RSSIquality()) {
            case 0:
                jsonWriteStr(configSetupJson, F("signal"), F("Уровень WiFi сигнала: <font color='red'>не подключено к роутеру</font>"));
                break;
            case 1:
                jsonWriteStr(configSetupJson, F("signal"), F("Уровень WiFi сигнала: <font color='red'>нет сигнала</font>"));
                break;
            case 2:
                jsonWriteStr(configSetupJson, F("signal"), F("Уровень WiFi сигнала: <font color='red'>очень низкий</font>"));
                break;
            case 3:
                jsonWriteStr(configSetupJson, F("signal"), F("Уровень WiFi сигнала: <font color='orange'>низкий</font>"));
                break;
            case 4:
                jsonWriteStr(configSetupJson, F("signal"), F("Уровень WiFi сигнала: <font color='green'>хороший</font>"));
                break;
            case 5:
                jsonWriteStr(configSetupJson, F("signal"), F("Уровень WiFi сигнала: <font color='green'>очень хороший</font>"));
                break;
            case 6:
                jsonWriteStr(configSetupJson, F("signal"), F("Уровень WiFi сигнала: <font color='green'>отличный</font>"));
                break;
            }
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
    if (myCountDown != nullptr) {
        for (unsigned int i = 0; i < myCountDown->size(); i++) {
            myCountDown->at(i).loop();
        }
    }
}