#include "Web.h"

#include "Class/NotAsync.h"
#include "Global.h"
#include "Init.h"
#include "ItemsList.h"
#include "RemoteOrdersUdp.h"
#include "SoftUART.h"
#include "Telegram.h"
#include "items/vLogging.h"

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
        if (request->hasArg(F("addItem"))) {
            addItem2(request->getParam("addItem")->value().toInt());
            request->redirect("/?set.device");
        }

        if (request->hasArg(F("addPreset"))) {
            addPreset2(request->getParam(F("addPreset"))->value().toInt());

            jsonWriteStr(configSetupJson, F("warning1"), F("<div style='margin-top:10px;margin-bottom:10px;'><font color='black'><p style='border: 1px solid #DCDCDC; border-radius: 3px; background-color: #ffc7c7; padding: 10px;'>Требуется перезагрузка</p></font></div>"));

            request->redirect(F("/?set.device"));
        }

        if (request->hasArg(F("delChoosingItems"))) {
            jsonWriteStr(configSetupJson, F("warning1"), F("<div style='margin-top:10px;margin-bottom:10px;'><font color='black'><p style='border: 1px solid #DCDCDC; border-radius: 3px; background-color: #ffc7c7; padding: 10px;'>Требуется перезагрузка</p></font></div>"));
            myNotAsyncActions->make(do_delChoosingItems);
            request->send(200);
        }

        if (request->hasArg(F("delAllItems"))) {
            delAllItems();
#ifdef EnableLogging
            cleanLogAndData();
#endif
            jsonWriteStr(configSetupJson, F("warning1"), F("<div style='margin-top:10px;margin-bottom:10px;'><font color='black'><p style='border: 1px solid #DCDCDC; border-radius: 3px; background-color: #ffc7c7; padding: 10px;'>Требуется перезагрузка</p></font></div>"));
            request->redirect(F("/?set.device"));
        }

        if (request->hasArg(F("saveItems"))) {
            myNotAsyncActions->make(do_deviceInit);
            savedFromWeb = true;
            request->send(200);
        }

        if (request->hasArg(F("scen"))) {
            bool value = request->getParam(F("scen"))->value().toInt();
            jsonWriteBool(configSetupJson, F("scen"), value);
            saveConfig();
            loadScenario();
            request->send(200);
        }

        if (request->hasArg(F("sceninit"))) {
            loadScenario();
            request->send(200);
        }

        if (request->hasArg(F("MqttIn"))) {
            bool value = request->getParam(F("MqttIn"))->value().toInt();
            jsonWriteBool(configSetupJson, "MqttIn", value);
            saveConfig();
            mqtt.subscribe((mqttPrefix + "/+/+/event").c_str());
            mqtt.subscribe((mqttPrefix + "/+/+/info").c_str());
            request->send(200);
        }

        if (request->hasArg(F("MqttOut"))) {
            bool value = request->getParam(F("MqttOut"))->value().toInt();
            jsonWriteBool(configSetupJson, F("MqttOut"), value);
            saveConfig();
            request->send(200);
        }

        if (request->hasArg(F("scenMqtt"))) {
            myNotAsyncActions->make(do_sendScenMQTT);
            request->send(200);
        }

        if (request->hasArg(F("cleanlog"))) {
#ifdef EnableLogging
            cleanLogAndData();
#endif
            request->send(200);
        }

        //==============================wifi settings=============================================
        if (request->hasArg(F("devname"))) {
            jsonWriteStr(configSetupJson, F("name"), request->getParam(F("devname"))->value());
            saveConfig();
            request->send(200);
        }

        if (request->hasArg(F("routerssid"))) {
            jsonWriteStr(configSetupJson, F("routerssid"), request->getParam(F("routerssid"))->value());
            saveConfig();
            request->send(200);
        }

        if (request->hasArg(F("routerpass"))) {
            jsonWriteStr(configSetupJson, F("routerpass"), request->getParam(F("routerpass"))->value());
            saveConfig();
            request->send(200);
        }

        if (request->hasArg(F("apssid"))) {
            jsonWriteStr(configSetupJson, F("apssid"), request->getParam(F("apssid"))->value());
            saveConfig();
            request->send(200, "text/text", "OK");
        }

        if (request->hasArg(F("appass"))) {
            jsonWriteStr(configSetupJson, F("appass"), request->getParam(F("appass"))->value());
            saveConfig();
            request->send(200);
        }

        if (request->hasArg(F("weblogin"))) {
            jsonWriteStr(configSetupJson, F("weblogin"), request->getParam(F("weblogin"))->value());
            saveConfig();
            request->send(200);
        }

        if (request->hasArg(F("webpass"))) {
            jsonWriteStr(configSetupJson, F("webpass"), request->getParam(F("webpass"))->value());
            saveConfig();
            request->send(200);
        }

        if (request->hasArg(F("timezone"))) {
            String timezoneStr = request->getParam(F("timezone"))->value();
            jsonWriteStr(configSetupJson, F("timezone"), timezoneStr);
            saveConfig();
            timeNow->setTimezone(timezoneStr.toInt());
            request->send(200);
        }

        if (request->hasArg(F("ntp"))) {
            String ntpStr = request->getParam(F("ntp"))->value();
            jsonWriteStr(configSetupJson, F("ntp"), ntpStr);
            saveConfig();
            timeNow->setNtpPool(ntpStr);
            request->send(200);
        }

        if (request->hasArg(F("blink"))) {
            bool value = request->getParam(F("blink"))->value().toInt();
            jsonWriteBool(configSetupJson, F("blink"), value);
            saveConfig();
            request->send(200);
        }

        if (request->hasArg(F("reqReset"))) {
            String tmp = "{}";
            jsonWriteStr(tmp, "title", F("<button class=\"close\" onclick=\"toggle('reset-block')\">×</button>Вы действительно хотите перезагрузить устройство?<a href=\"#\" class=\"btn btn-block btn-danger\" onclick=\"send_request(this, '/set?reset');setTimeout(function(){ location.href='/?set.device'; }, 15000);html('reset-block','<span class=loader></span>Идет перезагрузка устройства')\">Перезагрузить</a>"));
            jsonWriteStr(tmp, "class", "pop-up");
            request->send(200, "text/html", tmp);
        }

        if (request->hasArg(F("reset"))) {
            ESP.restart();
            request->send(200);
        }

        //==============================mqtt settings=============================================
        //primary
        if (request->hasArg(F("mqttServer"))) {
            jsonWriteStr(configSetupJson, F("mqttServer"), request->getParam(F("mqttServer"))->value());
            saveConfig();
            myNotAsyncActions->make(do_MQTTPARAMSCHANGED);
            request->send(200);
        }
        if (request->hasArg(F("mqttPort"))) {
            int port = (request->getParam(F("mqttPort"))->value()).toInt();
            jsonWriteInt(configSetupJson, F("mqttPort"), port);
            saveConfig();
            myNotAsyncActions->make(do_MQTTPARAMSCHANGED);
            request->send(200);
        }
        if (request->hasArg(F("mqttPrefix"))) {
            jsonWriteStr(configSetupJson, F("mqttPrefix"), request->getParam(F("mqttPrefix"))->value());
            saveConfig();
            myNotAsyncActions->make(do_MQTTPARAMSCHANGED);
            request->send(200);
        }
        if (request->hasArg(F("mqttUser"))) {
            jsonWriteStr(configSetupJson, F("mqttUser"), request->getParam(F("mqttUser"))->value());
            saveConfig();
            myNotAsyncActions->make(do_MQTTPARAMSCHANGED);
            request->send(200);
        }
        if (request->hasArg(F("mqttPass"))) {
            jsonWriteStr(configSetupJson, F("mqttPass"), request->getParam(F("mqttPass"))->value());
            saveConfig();
            myNotAsyncActions->make(do_MQTTPARAMSCHANGED);
            request->send(200);
        }
        //secondary
        if (request->hasArg(F("mqttServer2"))) {
            jsonWriteStr(configSetupJson, F("mqttServer2"), request->getParam(F("mqttServer2"))->value());
            saveConfig();
            myNotAsyncActions->make(do_MQTTPARAMSCHANGED);
            request->send(200);
        }
        if (request->hasArg(F("mqttPort2"))) {
            int port = (request->getParam(F("mqttPort2"))->value()).toInt();
            jsonWriteInt(configSetupJson, F("mqttPort2"), port);
            saveConfig();
            myNotAsyncActions->make(do_MQTTPARAMSCHANGED);
            request->send(200);
        }
        if (request->hasArg(F("mqttPrefix2"))) {
            jsonWriteStr(configSetupJson, F("mqttPrefix2"), request->getParam(F("mqttPrefix2"))->value());
            saveConfig();
            myNotAsyncActions->make(do_MQTTPARAMSCHANGED);
            request->send(200);
        }
        if (request->hasArg(F("mqttUser2"))) {
            jsonWriteStr(configSetupJson, F("mqttUser2"), request->getParam(F("mqttUser2"))->value());
            saveConfig();
            myNotAsyncActions->make(do_MQTTPARAMSCHANGED);
            request->send(200);
        }
        if (request->hasArg(F("mqttPass2"))) {
            jsonWriteStr(configSetupJson, F("mqttPass2"), request->getParam(F("mqttPass2"))->value());
            saveConfig();
            myNotAsyncActions->make(do_MQTTPARAMSCHANGED);
            request->send(200);
        }

        if (request->hasArg("mqttsend")) {
            //myNotAsyncActions->make(do_MQTTUDP);
            request->send(200);
        }

        if (request->hasArg("mqttcheck")) {
            String buf = "<button class=\"close\" onclick=\"toggle('my-block')\">×</button>" + getStateStr();

            String payload = "{}";
            jsonWriteStr(payload, "title", buf);
            jsonWriteStr(payload, "class", "pop-up");

            request->send(200, "text/html", payload);
        }

        //==============================telegram settings=============================================
        if (request->hasArg("telegramApi")) {
            jsonWriteStr(configSetupJson, "telegramApi", request->getParam("telegramApi")->value());
            saveConfig();
            request->send(200);
        }
        if (request->hasArg("autos")) {
            bool value = request->getParam("autos")->value().toInt();
            jsonWriteBool(configSetupJson, "autos", value);
            saveConfig();
            request->send(200);
        }
        if (request->hasArg("chatId")) {
            jsonWriteStr(configSetupJson, "chatId", request->getParam("chatId")->value());
            saveConfig();
            request->send(200);
        }
        if (request->hasArg("telegonof")) {
            bool value = request->getParam("telegonof")->value().toInt();
            jsonWriteBool(configSetupJson, "telegonof", value);
            saveConfig();
            request->send(200);
        }
        if (request->hasArg("teleginput")) {
            bool value = request->getParam("teleginput")->value().toInt();
            jsonWriteBool(configSetupJson, "teleginput", value);
            saveConfig();
            request->send(200);
        }

        //==============================utilities settings=============================================
        if (request->hasArg("i2c")) {
            myNotAsyncActions->make(do_BUSSCAN);
            request->redirect("/?set.utilities");
        }
        if (request->hasArg("uart")) {
            bool value = request->getParam("uart")->value().toInt();
            jsonWriteBool(configSetupJson, "uart", value);
            saveConfig();
#ifdef EnableUart
            uartInit();
#endif
            request->send(200);
        }
        if (request->hasArg("uartEvents")) {
            bool value = request->getParam("uartEvents")->value().toInt();
            jsonWriteBool(configSetupJson, "uartEvents", value);
            saveConfig();
            request->send(200);
        }
        if (request->hasArg("uartS")) {
            jsonWriteStr(configSetupJson, "uartS", request->getParam("uartS")->value());
            saveConfig();
#ifdef EnableUart
            uartInit();
#endif
            request->send(200);
        }
        if (request->hasArg("uartTX")) {
            jsonWriteStr(configSetupJson, "uartTX", request->getParam("uartTX")->value());
            saveConfig();
#ifdef EnableUart
            uartInit();
#endif
            request->send(200);
        }
        if (request->hasArg("uartRX")) {
            jsonWriteStr(configSetupJson, "uartRX", request->getParam("uartRX")->value());
            saveConfig();
#ifdef EnableUart
            uartInit();
#endif
            request->send(200);
        }

        //==============================developer settings=============================================
        if (request->hasArg("serverip")) {
            jsonWriteStr(configSetupJson, "serverip", request->getParam("serverip")->value());
            saveConfig();
            serverIP = jsonReadStr(configSetupJson, "serverip");
            request->send(200);
        }
        //set?order=button_1
        if (request->hasArg("order")) {
            String order = request->getParam("order")->value();
            order.replace("_", " ");
            orderBuf += order + ",";
            request->send(200);
        }

        if (request->hasArg("grafmax")) {
            int value = request->getParam("grafmax")->value().toInt();
            jsonWriteInt(configSetupJson, "grafmax", value);
            saveConfig();
            request->send(200);
        }

        //gate mode

        if (request->hasArg("gateAuto")) {
            bool value = request->getParam("gateAuto")->value().toInt();
            jsonWriteBool(configSetupJson, "gateAuto", value);
            saveConfig();
            request->send(200);
        }

    });

    //server.on("/del", HTTP_GET, [](AsyncWebServerRequest* request) {
    //    if (request->hasArg("file") && request->hasArg("line")) {
    //        String fileName = request->getParam("file")->value();
    //        Serial.println(fileName);
    //        int line = request->getParam("line")->value().toInt();
    //        Serial.println(line);
    //        myNotAsyncActions->make(do_delChoosingItems);
    //        request->redirect(F("/?set.device"));
    //    }
    //});

    server.on("/check", HTTP_GET, [](AsyncWebServerRequest* request) {
        myNotAsyncActions->make(do_GETLASTVERSION);
        SerialPrint("I", "Update", "firmware version: " + String(lastVersion));

        String msg = "";

        if (USE_OTA) {
            if (lastVersion == FIRMWARE_VERSION) {
                msg = F("Актуальная версия прошивки уже установлена.");
            } else if (lastVersion > FIRMWARE_VERSION) {
                msg = F("Новая версия прошивки<a href=\"#\" class=\"btn btn-block btn-danger\" onclick=\"send_request(this, '/upgrade');setTimeout(function(){ location.href='/?set.device'; }, 90000);html('my-block','<span class=loader></span>Идет обновление прошивки, после обновления страница  перезагрузится автоматически...')\">Установить</a>");
            } else if (lastVersion == -1) {
                msg = F("Cервер не найден. Попробуйте повторить позже...");
            } else if (lastVersion == -2) {
                msg = F("Устройство не подключено к роутеру!");
            } else if (lastVersion < FIRMWARE_VERSION) {
                msg = F("Ошибка версии. Попробуйте повторить позже...");
            }
        } else {
            msg = F("Обновление невозможно, память устройства 1 мб");
        }

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

    SerialPrint("I", F("Web"), F("WebAdmin Init"));
}

void setConfigParam(const char* param, const String& value) {
    SerialPrint("I", "Web", "set " + String(param) + ": " + value);
    jsonWriteStr(configSetupJson, param, value);
    saveConfig();
}