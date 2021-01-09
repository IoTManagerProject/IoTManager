#include "Init.h"

#include "BufferExecute.h"
#include "Class/LineParsing.h"
#include "Cmd.h"
#include "Global.h"
#include "items/vButtonOut.h"
#include "items/vCountDown.h"
#include "items/vImpulsOut.h"
#include "items/vInOutput.h"
#include "items/vLogging.h"
#include "items/vPwmOut.h"
#include "items/vSensorAnalog.h"
#include "items/vSensorBme280.h"
#include "items/vSensorBmp280.h"
#include "items/vSensorCcs811.h"
#include "items/vSensorDallas.h"
#include "items/vSensorDht.h"
#include "items/vSensorUltrasonic.h"
#include "items/vSensorUptime.h"

void loadConfig() {
    configSetupJson = readFile("config.json", 4096);
    configSetupJson.replace("\r\n", "");

    configStoreJson = readFile("store.json", 4096);
    configStoreJson.replace("\r\n", "");

    jsonWriteStr(configSetupJson, "warning1", "");
    jsonWriteStr(configSetupJson, "warning2", "");
    jsonWriteStr(configSetupJson, "warning3", "");

    jsonWriteStr(configSetupJson, "chipID", chipId);
    jsonWriteInt(configSetupJson, "firmware_version", FIRMWARE_VERSION);

    prex = jsonReadStr(configSetupJson, "mqttPrefix") + "/" + chipId;

    serverIP = jsonReadStr(configSetupJson, "serverip");

    SerialPrint("I", F("Conf"), F("Config Json Init"));
}

void espInit() {
    deviceInit();
    loadScenario();
    SerialPrint("I", F("esp"), F("esp Init"));
}

void deviceInit() {
    clearVectors();

#ifdef LAYOUT_IN_RAM
    all_widgets = "";
#else
    removeFile(String("layout.txt"));
#endif

    myLineParsing.clearErrors();

    fileCmdExecute(String(DEVICE_CONFIG_FILE));

    int errors = myLineParsing.getPinErrors();

    if (errors > 0) {
        jsonWriteStr(configSetupJson, F("warning3"), F("<div style='margin-top:10px;margin-bottom:10px;'><font color='black'><p style='border: 1px solid #DCDCDC; border-radius: 3px; background-color: #ffc7c7; padding: 10px;'>Обнаружен неверный номер пина</p></font></div>"));
    } else {
        jsonWriteStr(configSetupJson, F("warning3"), "");
    }
    //outcoming_date();
}

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
    SerialPrint("I", F("Uptime"), F("Uptime Init"));
}

void handle_uptime() {
    jsonWriteStr(configSetupJson, "uptime", timeNow->getUptime());
}

void clearVectors() {
    if (myLogging != nullptr) {
        myLogging->clear();
    }
    logging_KeyList = "";
    logging_EnterCounter = -1;

    if (myImpulsOut != nullptr) {
        myImpulsOut->clear();
    }
    impuls_KeyList = "";
    impuls_EnterCounter = -1;

    if (myCountDown != nullptr) {
        myCountDown->clear();
    }
    countDown_KeyList = "";
    countDown_EnterCounter = -1;

    if (myButtonOut != nullptr) {
        myButtonOut->clear();
    }
    buttonOut_KeyList = "";
    buttonOut_EnterCounter = -1;

    if (myInOutput != nullptr) {
        myInOutput->clear();
    }
    inOutput_KeyList = "";
    inOutput_EnterCounter = -1;

    if (myPwmOut != nullptr) {
        myPwmOut->clear();
    }
    pwmOut_KeyList = "";
    pwmOut_EnterCounter = -1;

    //==================================

    if (mySensorDallas2 != nullptr) {
        mySensorDallas2->clear();
    }

    if (mySensorUltrasonic != nullptr) {
        mySensorUltrasonic->clear();
    }

    if (mySensorAnalog != nullptr) {
        mySensorAnalog->clear();
    }

    if (mySensorDht != nullptr) {
        mySensorDht->clear();
    }

    if (mySensorBme280 != nullptr) {
        mySensorBme280->clear();
    }

    if (mySensorBmp280 != nullptr) {
        mySensorBmp280->clear();
    }

    if (mySensorCcs811 != nullptr) {
        mySensorCcs811->clear();
    }

    if (mySensorUptime != nullptr) {
        mySensorUptime->clear();
    }
}
