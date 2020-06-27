#include "CaptiveRequestHandler.h"
#include "Global.h"
#include "Utils/PresetUtils.h"

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
        if (request->hasArg(I2C_TAG)) {
            busScanFlag = true;
            busToScan = BS_I2C;
            request->redirect("/?set.utilities");
        }

        if (request->hasArg(ONE_WIRE_TAG)) {
            busScanFlag = true;
            busToScan = BS_ONE_WIRE;
            request->redirect("/?set.utilities");
        }

        if (request->hasArg("oneWirePin")) {
            //jsonWriteStr(configSetupJson, "oneWirePin", request->getParam("oneWirePin")->value());
            saveConfig();
            request->send(200, "text/text", "OK");
        }
    });
    //==============================upgrade settings=============================================
    server.on("/check", HTTP_GET, [](AsyncWebServerRequest* request) {
        upgrade_url = true;
        pm.info("firmware version: " + lastVersion);
        String tmp = "{}";
        int case_of_update;

        if (WiFi.status() != WL_CONNECTED) {
            lastVersion = "nowifi";
        }

        if (!FLASH_4MB) {
            lastVersion = "less";
        }

        if (lastVersion == FIRMWARE_VERSION) case_of_update = 1;
        if (lastVersion != FIRMWARE_VERSION) case_of_update = 2;
        if (lastVersion == "error") case_of_update = 3;
        if (lastVersion == "") case_of_update = 4;
        if (lastVersion == "less") case_of_update = 5;
        if (lastVersion == "nowifi") case_of_update = 6;
        if (lastVersion == "notsupported") case_of_update = 7;

        switch (case_of_update) {
            case 1: {
                jsonWriteStr(tmp, "title", "<button class=\"close\" onclick=\"toggle('my-block')\">×</button>Последняя версия прошивки уже установлена.");
                jsonWriteStr(tmp, "class", "pop-up");
            } break;

            case 2: {
                jsonWriteStr(tmp, "title", "<button class=\"close\" onclick=\"toggle('my-block')\">×</button>Имеется новая версия прошивки<a href=\"#\" class=\"btn btn-block btn-danger\" onclick=\"send_request(this, '/upgrade');setTimeout(function(){ location.href='/'; }, 120000);html('my-block','<span class=loader></span>Идет обновление прошивки, после обновления страница  перезагрузится автоматически...')\">Установить</a>");
                jsonWriteStr(tmp, "class", "pop-up");
            } break;

            case 3: {
                jsonWriteStr(tmp, "title", "<button class=\"close\" onclick=\"toggle('my-block')\">×</button>Ошибка... Cервер не найден. Попробуйте позже...");
                jsonWriteStr(tmp, "class", "pop-up");
            } break;

            case 4: {
                jsonWriteStr(tmp, "title", "<button class=\"close\" onclick=\"toggle('my-block')\">×</button>Нажмите на кнопку \"обновить прошивку\" повторно...");
                jsonWriteStr(tmp, "class", "pop-up");
                break;
            }

            case 5: {
                jsonWriteStr(tmp, "title", "<button class=\"close\" onclick=\"toggle('my-block')\">×</button>Обновление по воздуху не поддерживается, модуль имеет меньше 4 мб памяти...");
                jsonWriteStr(tmp, "class", "pop-up");
                break;
            }

            case 6: {
                jsonWriteStr(tmp, "title", "<button class=\"close\" onclick=\"toggle('my-block')\">×</button>Устройство не подключено к роутеру...");
                jsonWriteStr(tmp, "class", "pop-up");
                break;
            }

            case 7: {
                jsonWriteStr(tmp, "title", "<button class=\"close\" onclick=\"toggle('my-block')\">×</button>Обновление на новую версию возможно только через usb...");
                jsonWriteStr(tmp, "class", "pop-up");
                break;
            }
        }
        request->send(200, "text/text", tmp);
    });

    server.on("/upgrade", HTTP_GET, [](AsyncWebServerRequest* request) {
        upgrade = true;
        String tmp = "{}";
        request->send(200, "text/text", "ok");
    });
}