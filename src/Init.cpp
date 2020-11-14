#include "Init.h"
#include "BufferExecute.h"
#include "Cmd.h"
#include "Global.h"
#include "items/vLogging.h"
#include "items/vImpulsOut.h"
#include "items/vButtonOut.h"
#include "items/vSensorDallas.h"
#include "items/vInput.h"

void loadConfig() {
    configSetupJson = readFile("config.json", 4096);
    configSetupJson.replace("\r\n", "");

#ifdef SAVE_SETTINGS_TO_FLASH
    configLiveJson = readFile("live.json", 4096);
    configLiveJson.replace("\r\n", "");
#endif

    jsonWriteStr(configSetupJson, "chipID", chipId);
    jsonWriteInt(configSetupJson, "firmware_version", FIRMWARE_VERSION);

    prex = jsonReadStr(configSetupJson, "mqttPrefix") + "/" + chipId;

    Serial.println(configSetupJson);

    serverIP = jsonReadStr(configSetupJson, "serverip");
}

void all_init() {
    Device_init();
    loadScenario();
    Timer_countdown_init();
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
    if (myInput != nullptr) {
        myInput->clear();
    }
    input_KeyList = "";
    input_EnterCounter = -1;
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

//void handle_statistics() {
//    if (isNetworkActive()) {
//        String urls = "http://backup.privet.lv/visitors/?";
//        //-----------------------------------------------------------------
//        urls += WiFi.macAddress().c_str();
//        urls += "&";
//        //-----------------------------------------------------------------
//#ifdef ESP8266
//        urls += "iot-manager_esp8266";
//#endif
//#ifdef ESP32
//        urls += "iot-manager_esp32";
//#endif
//        urls += "&";
//#ifdef ESP8266
//        urls += ESP.getResetReason();
//#endif
//#ifdef ESP32
//        urls += "Power on";
//#endif
//        urls += "&";
//        urls += String(FIRMWARE_VERSION);
//        String stat = getURL(urls);
//    }
//}