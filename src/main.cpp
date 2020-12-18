
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
#include "items/vSensorUltrasonic.h"
#include "Telegram.h"
#include "SoftUART.h"
#include "FileSystem.h"

void not_async_actions();

Timings metric;
boolean initialized = false;

void setup() {
    Serial.begin(115200);
    Serial.flush();
    Serial.println();
    Serial.println(F("--------------started----------------"));

    myNotAsyncActions = new NotAsync(do_LAST);
    myScenario = new Scenario();

    //=========================================initialisation==============================================================
    setChipId();
    fileSystemInit();
    loadConfig();
#ifdef uartEnable
    uartInit();
#endif
    clockInit();
    timeInit();
    sensorsInit(); //Will be remooved
    itemsListInit();
    espInit();
    routerConnect();
#ifdef telegramEnable
    telegramInit();
#endif
    uptime_init();
    upgradeInit();
    HttpServer::init();
    web_init();
    initSt();
    busInit();
    wifiSignalInit();
#ifdef UDP_ENABLED
    asyncUdpInit();
#endif
#ifdef SSDP_ENABLED
    SsdpInit();
#endif
    getFSInfo();
    //esp_log_level_set("esp_littlefs", ESP_LOG_NONE);
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

    myNotAsyncActions->loop();
    ts.update();

#ifdef telegramEnable
    handleTelegram();
#endif

#ifdef uartEnable
    uartHandle();
#endif

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
    if (mySensorUltrasonic != nullptr) {
        for (unsigned int i = 0; i < mySensorUltrasonic->size(); i++) {
            mySensorUltrasonic->at(i).loop();
        }
    }
    if (myCountDown != nullptr) {
        for (unsigned int i = 0; i < myCountDown->size(); i++) {
            myCountDown->at(i).loop();
        }
    }
}