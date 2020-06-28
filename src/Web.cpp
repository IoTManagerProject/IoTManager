#include "Global.h"

static const char* MODULE = "Web";

static const uint8_t MIN_PRESET = 1;
static const uint8_t MAX_PRESET = 21;

bool parseRequestForPreset(AsyncWebServerRequest* request, uint8_t& preset) {
    if (request->hasArg("preset")) {
        preset = request->getParam("preset")->value().toInt();
        return true;
    }
    return false;
}

void web_init() {
    // server.addHandler(new CaptiveRequestHandler(jsonReadStr(configSetupJson, "name").c_str())).setFilter(ON_AP_FILTER);

    server.on("/set", HTTP_GET, [](AsyncWebServerRequest* request) {
        uint8_t preset;
        if (parseRequestForPreset(request, preset)) {
            pm.info("activate #" + String(preset, DEC));
            String configFile = DEVICE_CONFIG_FILE;
            String scenarioFile = DEVICE_SCENARIO_FILE;
            copyFile(getConfigFile(preset, CT_CONFIG), configFile);
            copyFile(getConfigFile(preset, CT_SCENARIO), scenarioFile);
            Device_init();
            loadScenario();
            request->redirect("/?set.device");
        }

        //--------------------------------------------------------------------------------
        if (request->hasArg("devinit")) {
            Device_init();
            request->send(200, "text/text", "OK");
        }
        //--------------------------------------------------------------------------------
        if (request->hasArg("scen")) {
            String value = request->getParam("scen")->value();
            if (value == "0") {
                jsonWriteStr(configSetupJson, "scen", value);
                saveConfig();
                loadScenario();
            }
            if (value == "1") {
                jsonWriteStr(configSetupJson, "scen", value);
                saveConfig();
                loadScenario();
            }
            request->send(200, "text/text", "OK");
        }
        //--------------------------------------------------------------------------------
        if (request->hasArg("sceninit")) {
            loadScenario();
            request->send(200, "text/text", "OK");
        }
        //--------------------------------------------------------------------------------
#ifdef LOGGING_ENABLED
        if (request->hasArg("cleanlog")) {
            clean_log_date();
            request->send(200, "text/text", "OK");
        }
#endif
        //==============================udp settings=============================================
        if (request->hasArg("udponoff")) {
            String value = request->getParam("udponoff")->value();
            if (value == "0") {
                jsonWriteStr(configSetupJson, "udponoff", value);
                saveConfig();
                loadScenario();
            }
            if (value == "1") {
                jsonWriteStr(configSetupJson, "udponoff", value);
                saveConfig();
                loadScenario();
            }
            request->send(200, "text/text", "OK");
        }
        //--------------------------------------------------------------------------------
        if (request->hasArg("updatelist")) {
            removeFile("/dev.csv");
            addFile("dev.csv", "device id;device name;ip address");
            request->redirect("/?set.udp");
        }
        //--------------------------------------------------------------------------------
        if (request->hasArg("updatepage")) {
            request->redirect("/?set.udp");
        }
        //--------------------------------------------------------------------------------
        if (request->hasArg("devname")) {
            jsonWriteStr(configSetupJson, "name", request->getParam("devname")->value());
            saveConfig();
            request->send(200, "text/text", "OK");
        }
        //==============================wifi settings=============================================
        if (request->hasArg("routerssid")) {
            jsonWriteStr(configSetupJson, "routerssid", request->getParam("routerssid")->value());
            saveConfig();
            request->send(200, "text/text", "OK");
        }
        if (request->hasArg("routerpass")) {
            jsonWriteStr(configSetupJson, "routerpass", request->getParam("routerpass")->value());
            saveConfig();
            request->send(200, "text/text", "OK");
        }
        //--------------------------------------------------------------------------------
        if (request->hasArg("apssid")) {
            jsonWriteStr(configSetupJson, "apssid", request->getParam("apssid")->value());
            saveConfig();
            request->send(200, "text/text", "OK");
        }
        if (request->hasArg("appass")) {
            jsonWriteStr(configSetupJson, "appass", request->getParam("appass")->value());
            saveConfig();
            request->send(200, "text/text", "OK");
        }
        //--------------------------------------------------------------------------------
        if (request->hasArg("weblogin")) {
            jsonWriteStr(configSetupJson, "weblogin", request->getParam("weblogin")->value());
            saveConfig();
            request->send(200, "text/text", "OK");
        }
        if (request->hasArg("webpass")) {
            jsonWriteStr(configSetupJson, "webpass", request->getParam("webpass")->value());
            saveConfig();
            request->send(200, "text/text", "OK");
        }
        //--------------------------------------------------------------------------------
        if (request->hasArg("timezone")) {
            String timezoneStr = request->getParam("timezone")->value();
            jsonWriteStr(configSetupJson, "timezone", timezoneStr);
            saveConfig();
            timeNow->setTimezone(timezoneStr.toInt());
            request->send(200, "text/text", "OK");
        }
        if (request->hasArg("ntp")) {
            String ntpStr = request->getParam("ntp")->value();
            jsonWriteStr(configSetupJson, "ntp", ntpStr);
            saveConfig();
            timeNow->setNtpPool(ntpStr);
            request->send(200, "text/text", "OK");
        }
        //--------------------------------------------------------------------------------
        if (request->hasArg("device")) {
            if (request->getParam("device")->value() == "ok") ESP.restart();
            request->send(200, "text/text", "OK");
        }
        //--------------------------------------------------------------------------------
        if (request->hasArg("blink")) {
            String value = request->getParam("blink")->value();
            if (value == "0") {
                jsonWriteStr(configSetupJson, "blink", value);
                saveConfig();
            }
            if (value == "1") {
                jsonWriteStr(configSetupJson, "blink", value);
                saveConfig();
            }
            request->send(200, "text/text", "OK");
        }
        //==============================mqtt settings=============================================
        if (request->hasArg("mqttServer")) {
            jsonWriteStr(configSetupJson, "mqttServer", request->getParam("mqttServer")->value());
            saveConfig();
            mqttParamsChanged = true;
            request->send(200, "text/text", "ok");
        }
        if (request->hasArg("mqttPort")) {
            int port = (request->getParam("mqttPort")->value()).toInt();
            jsonWriteInt(configSetupJson, "mqttPort", port);
            saveConfig();
            mqttParamsChanged = true;
            request->send(200, "text/text", "ok");
        }
        if (request->hasArg("mqttPrefix")) {
            jsonWriteStr(configSetupJson, "mqttPrefix", request->getParam("mqttPrefix")->value());
            saveConfig();
            mqttParamsChanged = true;
            request->send(200, "text/text", "ok");
        }
        if (request->hasArg("mqttUser")) {
            jsonWriteStr(configSetupJson, "mqttUser", request->getParam("mqttUser")->value());
            saveConfig();
            mqttParamsChanged = true;
            request->send(200, "text/text", "ok");
        }
        if (request->hasArg("mqttPass")) {
            jsonWriteStr(configSetupJson, "mqttPass", request->getParam("mqttPass")->value());
            saveConfig();
            mqttParamsChanged = true;
            request->send(200, "text/text", "ok");
        }
        //--------------------------------------------------------------------------------
        if (request->hasArg("mqttsend")) {
            mqtt_send_settings_to_udp = true;
            request->send(200, "text/text", "ok");
        }
        //--------------------------------------------------------------------------------
        if (request->hasArg("mqttcheck")) {
            String buf = "{}";
            String payload = "<button class=\"close\" onclick=\"toggle('my-block')\">×</button>" + MqttClient::getStateStr();
            jsonWriteStr(buf, "title", payload);
            jsonWriteStr(buf, "class", "pop-up");

            request->send(200, "text/text", buf);
        }
        //==============================push settings=============================================
#ifdef PUSH_ENABLED
        if (request->hasArg("pushingboxid")) {
            jsonWriteStr(configSetupJson, "pushingboxid", request->getParam("pushingboxid")->value());
            saveConfig();
            request->send(200, "text/text", "ok");
        }
#endif
        //==============================utilities settings=============================================
        if (request->hasArg(TAG_I2C)) {
            busScanFlag = true;
            busToScan = BS_I2C;
            request->redirect("/?set.utilities");
        }

        if (request->hasArg(TAG_ONE_WIRE)) {
            busScanFlag = true;
            busToScan = BS_ONE_WIRE;
            if (request->hasParam(TAG_ONE_WIRE_PIN)) {
                setConfigParam(TAG_ONE_WIRE_PIN, request->getParam(TAG_ONE_WIRE_PIN)->value());
            }
            request->redirect("/?set.utilities");
        }

        if (request->hasArg(TAG_ONE_WIRE_PIN)) {
            setConfigParam(TAG_ONE_WIRE_PIN, request->getParam(TAG_ONE_WIRE_PIN)->value());
            request->send(200);
        }
    });
    //==============================upgrade settings=============================================
    server.on("/check", HTTP_GET, [](AsyncWebServerRequest* request) {
        checkUpdatesFlag = true;
        pm.info("firmware version: " + lastVersion);

        if (!FLASH_4MB) {
            lastVersion = "less";
        } else if (isNetworkActive()) {
            lastVersion = "nowifi";
        }

        String msg = "";
        if (lastVersion == FIRMWARE_VERSION) {
            msg = F("Актуальная версия прошивки уже установлена.");
        } else if (lastVersion != FIRMWARE_VERSION) {
            msg = F("Новая версия прошивки<a href=\"#\" class=\"btn btn-block btn-danger\" onclick=\"send_request(this, '/upgrade');setTimeout(function(){ location.href='/'; }, 120000);html('my-block','<span class=loader></span>Идет обновление прошивки, после обновления страница  перезагрузится автоматически...')\">Установить</a>");
        } else if (lastVersion == "error") {
            msg = F("Cервер не найден. Попробуйте повторить позже...");
        } else if (lastVersion == "") {
            msg = F("Нажмите на кнопку \"обновить прошивку\" повторно...");
        } else if (lastVersion == "less") {
            msg = F("Обновление \"по воздуху\" не поддерживается!");
        } else if (lastVersion == "nowifi") {
            msg = F("Устройство не подключено к роутеру!");
        } else if (lastVersion == "notsupported") {
            msg = F("Обновление возможно только через usb!");
        }
        String tmp = "{}";
        jsonWriteStr(tmp, "title", "<button class=\"close\" onclick=\"toggle('my-block')\">×</button>" + msg);
        jsonWriteStr(tmp, "class", "pop-up");
        request->send(200, "text/html", tmp);
    });

    server.on("/upgrade", HTTP_GET, [](AsyncWebServerRequest* request) {
        updateFlag = true;
        request->send(200, "text/html");
    });
}