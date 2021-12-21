
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
#include "MySensorsDataParse.h"
#include "RemoteOrdersUdp.h"
#include "SoftUART.h"
#include "Telegram.h"
#include "Tests.h"
#include "Utils/StatUtils.h"
#include "Utils/Timings.h"
#include "Utils/WebUtils.h"
#include "WebServer.h"
#include "WebSocket.h"
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
#include "items/vSensorNode.h"
#include "items/vSensorPzem.h"
#include "items/vSensorUltrasonic.h"
#include "items/vSensorUptime.h"
//=========================================
#include "SetupESP.h"
#include "WebServer.h"
#include "ws.h"

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
#ifdef EnableUart
    uartInit();
#endif
    clockInit();
    timeInit();
    itemsListInit();
    espInit();
    routerConnect();
#ifdef EnableTelegram
    telegramInit();
#endif
    uptime_init();
    upgradeInit();
    HttpServerinit();
    HttpServerinitWS();
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

    testsPerform();

    just_load = false;
    initialized = true;

    // const int capacity = JSON_OBJECT_SIZE(35);
    // SerialPrint("I", F("Test"), String(capacity));

    ts.add(
        MYTEST, 5000, [&](void*) {
            // ws.textAll("test msg from esp");
        },
        nullptr, true);

    // setupESP();

    myWebSockets = new WebSockets;

    wsInit();

    SerialPrint("I", F("System"), F("✔ Initialization completed"));
}

void loop() {
    if (!initialized) {
        return;
    }

    if (myWebSockets != nullptr) {
        myWebSockets->loop();
    }

    testLoop();

    // if (wsSetupFlag) {
    // wsSetupFlag = false;
    // wsSendSetup();
    //}

    // loopWsExecute();

#ifdef OTA_UPDATES_ENABLED
    ArduinoOTA.handle();
#endif
#ifdef WS_enable
    ws.cleanupClients();
#endif
    timeNow->loop();
    mqttLoop();

    myScenario->loop2();
    loopCmdExecute();

    myNotAsyncActions->loop();
    ts.update();

#ifdef EnableTelegram
    handleTelegram();
#endif

#ifdef EnableUart
    uartHandle();
#endif
#ifdef MYSENSORS
    loopMySensorsExecute();
#endif

#ifdef EnableCountDown
    if (myCountDown != nullptr) {
        for (unsigned int i = 0; i < myCountDown->size(); i++) {
            myCountDown->at(i).loop();
        }
    }
#endif
#ifdef EnableImpulsOut
    if (myImpulsOut != nullptr) {
        for (unsigned int i = 0; i < myImpulsOut->size(); i++) {
            myImpulsOut->at(i).loop();
        }
    }
#endif
#ifdef EnableLogging
    if (myLogging != nullptr) {
        for (unsigned int i = 0; i < myLogging->size(); i++) {
            myLogging->at(i).loop();
        }
    }
#endif
#ifdef EnableSensorDallas
    if (mySensorDallas2 != nullptr) {
        for (unsigned int i = 0; i < mySensorDallas2->size(); i++) {
            mySensorDallas2->at(i).loop();
        }
    }
#endif
#ifdef EnableSensorUltrasonic
    if (mySensorUltrasonic != nullptr) {
        for (unsigned int i = 0; i < mySensorUltrasonic->size(); i++) {
            mySensorUltrasonic->at(i).loop();
        }
    }
#endif

#ifdef EnableSensorAnalog
    if (mySensorAnalog != nullptr) {
        for (unsigned int i = 0; i < mySensorAnalog->size(); i++) {
            mySensorAnalog->at(i).loop();
        }
    }
#endif
#ifdef EnableSensorDht
    if (mySensorDht != nullptr) {
        for (unsigned int i = 0; i < mySensorDht->size(); i++) {
            mySensorDht->at(i).loop();
        }
    }
#endif
#ifdef EnableSensorBme280
    if (mySensorBme280 != nullptr) {
        for (unsigned int i = 0; i < mySensorBme280->size(); i++) {
            mySensorBme280->at(i).loop();
        }
    }
#endif
#ifdef EnableSensorBmp280
    if (mySensorBmp280 != nullptr) {
        for (unsigned int i = 0; i < mySensorBmp280->size(); i++) {
            mySensorBmp280->at(i).loop();
        }
    }
#endif
#ifdef EnableSensorCcs811
    if (mySensorCcs811 != nullptr) {
        for (unsigned int i = 0; i < mySensorCcs811->size(); i++) {
            mySensorCcs811->at(i).loop();
        }
    }
#endif
#ifdef EnableSensorPzem
    if (mySensorPzem != nullptr) {
        for (unsigned int i = 0; i < mySensorPzem->size(); i++) {
            mySensorPzem->at(i).loop();
        }
    }
#endif
#ifdef EnableSensorUptime
    if (mySensorUptime != nullptr) {
        for (unsigned int i = 0; i < mySensorUptime->size(); i++) {
            mySensorUptime->at(i).loop();
        }
    }
#endif
#ifdef EnableSensorNode
    if (mySensorNode != nullptr) {
        for (unsigned int i = 0; i < mySensorNode->size(); i++) {
            mySensorNode->at(i).loop();
        }
    }
#endif
#ifdef EnableButtonIn
    myButtonIn.loop();
#endif
}