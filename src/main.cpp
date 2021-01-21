
#include <SSDP.h>

#include "BufferExecute.h"
#include "Bus.h"
#include "Class/CallBackTest.h"
#include "Class/NotAsync.h"
#include "Class/ScenarioClass3.h"
#include "Cmd.h"
#include "FileSystem.h"
#include "Global.h"
#include "Init.h"
#include "ItemsList.h"
#include "RemoteOrdersUdp.h"
#include "SoftUART.h"
#include "Telegram.h"
#include "Tests.h"
#include "Utils/StatUtils.h"
#include "Utils/Timings.h"
#include "Utils/WebUtils.h"
#include "items/ButtonInClass.h"
#include "items/vCountDown.h"
#include "items/vImpulsOut.h"
#include "items/vLogging.h"
#include "items/vSensorAnalog.h"
#include "items/vSensorBme280.h"
#include "items/vSensorBmp280.h"
#include "items/vSensorCcs811.h"
#include "items/vSensorDallas.h"
#include "items/vSensorDht.h"
#include "items/vSensorPzem.h"
#include "items/vSensorUltrasonic.h"
#include "items/vSensorUptime.h"

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
#ifdef ESP8266
    getFSInfo();
#endif

    testsPerform();

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
#ifdef MYSENSORS
    loopMySensorsExecute();
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
    if (mySensorAnalog != nullptr) {
        for (unsigned int i = 0; i < mySensorAnalog->size(); i++) {
            mySensorAnalog->at(i).loop();
        }
    }
    if (mySensorDht != nullptr) {
        for (unsigned int i = 0; i < mySensorDht->size(); i++) {
            mySensorDht->at(i).loop();
        }
    }
    if (mySensorBme280 != nullptr) {
        for (unsigned int i = 0; i < mySensorBme280->size(); i++) {
            mySensorBme280->at(i).loop();
        }
    }
    if (mySensorBmp280 != nullptr) {
        for (unsigned int i = 0; i < mySensorBmp280->size(); i++) {
            mySensorBmp280->at(i).loop();
        }
    }
    if (mySensorBmp280 != nullptr) {
        for (unsigned int i = 0; i < mySensorBmp280->size(); i++) {
            mySensorBmp280->at(i).loop();
        }
    }
    if (mySensorCcs811 != nullptr) {
        for (unsigned int i = 0; i < mySensorCcs811->size(); i++) {
            mySensorCcs811->at(i).loop();
        }
    }
    if (mySensorPzem != nullptr) {
        for (unsigned int i = 0; i < mySensorPzem->size(); i++) {
            mySensorPzem->at(i).loop();
        }
    }
    if (mySensorUptime != nullptr) {
        for (unsigned int i = 0; i < mySensorUptime->size(); i++) {
            mySensorUptime->at(i).loop();
        }
    }
}