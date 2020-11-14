#include "Web.h"
#include "Class/NotAsync.h"
#include "Global.h"
#include "Init.h"
#include "ItemsList.h"
#include "items/vLogging.h"
#include "Telegram.h"

bool parseRequestForPreset(AsyncWebServerRequest* request, uint8_t& preset) {
    if (request->hasArg("preset")) {
        preset = request->getParam("preset")->value().toInt();
        return true;
    }
    return false;
}

void web_init() {
    server.on("/set", HTTP_GET, [](AsyncWebServerRequest* request) {
        //==============================set.device.json====================================================================================================
        if (request->hasArg("addItem")) {
            String name = request->getParam("addItem")->value();
            addItem(name);
            request->redirect("/?set.device");
        }

        if (request->hasArg("addPreset")) {
            String name = request->getParam("addPreset")->value();
            addPreset(name);
            request->redirect("/?set.device");
        }

        if (request->hasArg("delChoosingItems")) {
            myNotAsyncActions->make(do_delChoosingItems);
            request->send(200);
        }

        if (request->hasArg("delAllItems")) {
            delAllItems();
            request->redirect("/?set.device");
        }

        if (request->hasArg("saveItems")) {
            myNotAsyncActions->make(do_deviceInit);
            request->send(200);
        }

        if (request->hasArg("scen")) {
            bool value = request->getParam("scen")->value().toInt();
            jsonWriteBool(configSetupJson, "scen", value);
            saveConfig();
            loadScenario();
            request->send(200);
        }

        if (request->hasArg("sceninit")) {
            loadScenario();
            request->send(200);
        }

#ifdef LOGGING_ENABLED
        if (request->hasArg("cleanlog")) {
            cleanLogAndData();
            request->send(200);
        }
#endif

        //==============================wifi settings=============================================
        if (request->hasArg("devname")) {
            jsonWriteStr(configSetupJson, "name", request->getParam("devname")->value());
            saveConfig();
            request->send(200);
        }

        if (request->hasArg("routerssid")) {
            jsonWriteStr(configSetupJson, "routerssid", request->getParam("routerssid")->value());
            saveConfig();
            request->send(200);
        }

        if (request->hasArg("routerpass")) {
            jsonWriteStr(configSetupJson, "routerpass", request->getParam("routerpass")->value());
            saveConfig();
            request->send(200);
        }

        if (request->hasArg("apssid")) {
            jsonWriteStr(configSetupJson, "apssid", request->getParam("apssid")->value());
            saveConfig();
            request->send(200, "text/text", "OK");
        }

        if (request->hasArg("appass")) {
            jsonWriteStr(configSetupJson, "appass", request->getParam("appass")->value());
            saveConfig();
            request->send(200);
        }

        if (request->hasArg("weblogin")) {
            jsonWriteStr(configSetupJson, "weblogin", request->getParam("weblogin")->value());
            saveConfig();
            request->send(200);
        }

        if (request->hasArg("webpass")) {
            jsonWriteStr(configSetupJson, "webpass", request->getParam("webpass")->value());
            saveConfig();
            request->send(200);
        }

        if (request->hasArg("timezone")) {
            String timezoneStr = request->getParam("timezone")->value();
            jsonWriteStr(configSetupJson, "timezone", timezoneStr);
            saveConfig();
            timeNow->setTimezone(timezoneStr.toInt());
            request->send(200);
        }

        if (request->hasArg("ntp")) {
            String ntpStr = request->getParam("ntp")->value();
            jsonWriteStr(configSetupJson, "ntp", ntpStr);
            saveConfig();
            timeNow->setNtpPool(ntpStr);
            request->send(200);
        }

        if (request->hasArg("blink")) {
            bool value = request->getParam("blink")->value().toInt();
            jsonWriteBool(configSetupJson, "blink", value);
            saveConfig();
            request->send(200);
        }

        if (request->hasArg("device")) {
            if (request->getParam("device")->value() == "ok") {
                ESP.restart();
            }
            request->send(200);
        }

        if (request->hasArg("test")) {
            if (request->getParam("test")->value() == "ok") {
                Serial.println("test pass");
            }
            request->send(200);
        }

        //==============================mqtt settings=============================================

        if (request->hasArg("mqttServer")) {
            jsonWriteStr(configSetupJson, "mqttServer", request->getParam("mqttServer")->value());
            saveConfig();
            myNotAsyncActions->make(do_MQTTPARAMSCHANGED);
            request->send(200);
        }
        if (request->hasArg("mqttPort")) {
            int port = (request->getParam("mqttPort")->value()).toInt();
            jsonWriteInt(configSetupJson, "mqttPort", port);
            saveConfig();
            myNotAsyncActions->make(do_MQTTPARAMSCHANGED);
            request->send(200);
        }
        if (request->hasArg("mqttPrefix")) {
            jsonWriteStr(configSetupJson, "mqttPrefix", request->getParam("mqttPrefix")->value());
            saveConfig();
            myNotAsyncActions->make(do_MQTTPARAMSCHANGED);
            request->send(200);
        }
        if (request->hasArg("mqttUser")) {
            jsonWriteStr(configSetupJson, "mqttUser", request->getParam("mqttUser")->value());
            saveConfig();
            myNotAsyncActions->make(do_MQTTPARAMSCHANGED);
            request->send(200);
        }
        if (request->hasArg("mqttPass")) {
            jsonWriteStr(configSetupJson, "mqttPass", request->getParam("mqttPass")->value());
            saveConfig();
            myNotAsyncActions->make(do_MQTTPARAMSCHANGED);
            request->send(200);
        }

        if (request->hasArg("mqttsend")) {
            myNotAsyncActions->make(do_MQTTUDP);
            request->send(200);
        }

        if (request->hasArg("mqttcheck")) {
            String buf = "<button class=\"close\" onclick=\"toggle('my-block')\">×</button>" + getStateStr();

            String payload = "{}";
            jsonWriteStr(payload, "title", buf);
            jsonWriteStr(payload, "class", "pop-up");

            request->send(200, "text/html", payload);
        }

        //==============================push settings=============================================
        if (request->hasArg("telegramApi")) {
            jsonWriteStr(configSetupJson, "telegramApi", request->getParam("telegramApi")->value());
            //telegramInit();
            saveConfig();
            request->send(200);
        }
        if (request->hasArg("telegonof")) {
            bool value = request->getParam("telegonof")->value().toInt();
            jsonWriteBool(configSetupJson, "telegonof", value);
            //telegramInit();
            saveConfig();
            request->send(200);
        }

        //==============================utilities settings=============================================
        if (request->hasArg("i2c")) {
            myNotAsyncActions->make(do_BUSSCAN);
            request->redirect("/?set.utilities");
        }

        //==============================developer settings=============================================
        if (request->hasArg("serverip")) {
            jsonWriteStr(configSetupJson, "serverip", request->getParam("serverip")->value());
            saveConfig();
            serverIP = jsonReadStr(configSetupJson, "serverip");
            request->send(200);
        }
        });

    //==============================list of items=====================================================
    //server.on("/del", HTTP_GET, [](AsyncWebServerRequest* request) {
    //    if (request->hasArg("file")) {
    //        itemsFile = request->getParam("file")->value();
    //    }
    //    if (request->hasArg("line")) {
    //        itemsLine = request->getParam("line")->value();
    //    }
    //    delElementFlag = true;
    //    Device_init();
    //    request->redirect("/?setn.device");
    //});

    /*
    * Check
    */
    server.on("/check", HTTP_GET, [](AsyncWebServerRequest* request) {
        myNotAsyncActions->make(do_GETLASTVERSION);
        SerialPrint("I", "Update", "firmware version: " + String(lastVersion));

        String msg = "";
        if (lastVersion == FIRMWARE_VERSION) {
            msg = F("Актуальная версия прошивки уже установлена.");
        }
        else if (lastVersion > FIRMWARE_VERSION) {
            msg = F("Новая версия прошивки<a href=\"#\" class=\"btn btn-block btn-danger\" onclick=\"send_request(this, '/upgrade');setTimeout(function(){ location.href='/?set.device'; }, 90000);html('my-block','<span class=loader></span>Идет обновление прошивки, после обновления страница  перезагрузится автоматически...')\">Установить</a>");
        }
        else if (lastVersion == -1) {
            msg = F("Cервер не найден. Попробуйте повторить позже...");
        }
        else if (lastVersion == -2) {
            msg = F("Устройство не подключено к роутеру!");
        }
        else if (lastVersion < FIRMWARE_VERSION) {
            msg = F("Ошибка версии. Попробуйте повторить позже...");
        }

        // else if (lastVersion == "") {
        //msg = F("Нажмите на кнопку \"обновить прошивку\" повторно...");
        //} else if (lastVersion == "less") {
        //msg = F("Обновление \"по воздуху\" не поддерживается!");
        //} else if (lastVersion == "notsupported") {
        //   msg = F("Обновление возможно только через usb!");
        //}

        String tmp = "{}";
        jsonWriteStr(tmp, "title", "<button class=\"close\" onclick=\"toggle('my-block')\">×</button>" + msg);
        jsonWriteStr(tmp, "class", "pop-up");
        request->send(200, "text/html", tmp);
        });

    /*
    * Upgrade
    */
    server.on("/upgrade", HTTP_GET, [](AsyncWebServerRequest* request) {
        myNotAsyncActions->make(do_UPGRADE);
        request->send(200, "text/html");
        });
}

void setConfigParam(const char* param, const String& value) {
    SerialPrint("I", "Web", "set " + String(param) + ": " + value);
    jsonWriteStr(configSetupJson, param, value);
    saveConfig();
}