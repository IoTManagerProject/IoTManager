#include "Init.h"
#include "BufferExecute.h"
#include "Cmd.h"
#include "Global.h"
#include "items/vLogging.h"
#include "items/vImpulsOut.h"
#include "items/vButtonOut.h"
#include "items/vSensorDallas.h"
#include "items/vInOutput.h"
#include "items/vPwmOut.h"
#include "items/vCountDown.h"

void loadConfig() {
    configSetupJson = readFile("config.json", 4096);
    configSetupJson.replace("\r\n", "");

    configStoreJson = readFile("store.json", 4096);
    configStoreJson.replace("\r\n", "");

    jsonWriteStr(configSetupJson, "warning1", "");
    jsonWriteStr(configSetupJson, "warning2", "");

    jsonWriteStr(configSetupJson, "chipID", chipId);
    jsonWriteInt(configSetupJson, "firmware_version", FIRMWARE_VERSION);

    prex = jsonReadStr(configSetupJson, "mqttPrefix") + "/" + chipId;

    Serial.println(configSetupJson);

    serverIP = jsonReadStr(configSetupJson, "serverip");
}

void all_init() {
    Device_init();
    loadScenario();
}

void Device_init() {

    sensorReadingMap10sec = "";

    //======clear dallas params======
    if (mySensorDallas2 != nullptr) {
        mySensorDallas2->clear();
    }
    //======clear logging params======
    if (myLogging != nullptr) {
        myLogging->clear();
    }
    loggingKeyList = "";
    //======clear impuls params=======
    if (myImpulsOut != nullptr) {
        myImpulsOut->clear();
    }
    impuls_KeyList = "";
    impuls_EnterCounter = -1;
    //======clear buttonOut params=======
    if (myButtonOut != nullptr) {
        myButtonOut->clear();
    }
    buttonOut_KeyList = "";
    buttonOut_EnterCounter = -1;
    //======clear input params=======
    if (myInOutput != nullptr) {
        myInOutput->clear();
    }
    inOutput_KeyList = "";
    inOutput_EnterCounter = -1;
    //======clear pwm params=======
    if (myPwmOut != nullptr) {
        myPwmOut->clear();
    }
    pwmOut_KeyList = "";
    pwmOut_EnterCounter = -1;
    //===================================
    if (myCountDown != nullptr) {
        myCountDown->clear();
    }
    countDown_KeyList = "";
    countDown_EnterCounter = -1;
    //===================================


#ifdef LAYOUT_IN_RAM
    all_widgets = "";
#else
    removeFile(String("layout.txt"));
#endif

    fileCmdExecute(String(DEVICE_CONFIG_FILE));
    //outcoming_date();
}
//-------------------------------сценарии-----------------------------------------------------

void loadScenario() {
    if (jsonReadStr(configSetupJson, "scen") == "1") {
        scenario = readFile(String(DEVICE_SCENARIO_FILE), 2048);
    }
}

void uptime_init() {
    ts.add(
        UPTIME, 5000, [&](void*) {
            handle_uptime();
        },
        nullptr, true);
}

void handle_uptime() {
    jsonWriteStr(configSetupJson, "uptime", timeNow->getUptime());
}

