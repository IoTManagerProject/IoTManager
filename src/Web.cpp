#include "Global.h"

static const char* MODULE = "Web";

bool parseRequestForPreset(AsyncWebServerRequest* request, uint8_t& preset) {
    if (request->hasArg("preset")) {
        preset = request->getParam("preset")->value().toInt();
        return true;
    }
    return false;
}

void web_init() {
    // dnsServer.start(53, "*", WiFi.softAPIP());
    // server.addHandler(new CaptiveRequestHandler(jsonReadStr(configSetupJson, "name").c_str())).setFilter(ON_AP_FILTER);

    server.on("/restart", HTTP_GET, [](AsyncWebServerRequest* request) {
        if (request->hasArg("device")) {
            if (request->getParam("device")->value() == "ok") {
                ESP.restart();
            }
            request->send(200);
        };
    });

    server.on("/set", HTTP_GET, [](AsyncWebServerRequest* request) {
        uint8_t preset;
        if (parseRequestForPreset(request, preset)) {
            pm.info("activate #" + String(preset, DEC));
            String configFile = DEVICE_CONFIG_FILE;
            String scenarioFile = DEVICE_SCENARIO_FILE;
            copyFile(getConfigFile(preset, CT_CONFIG), configFile);
            copyFile(getConfigFile(preset, CT_SCENARIO), scenarioFile);
            Device_init();
            Scenario::load();
            request->redirect("/?set.device");
        } else if (request->hasArg("devinit")) {
            Device_init();
            request->send(200);
        } else if (request->hasArg("scen")) {
            bool value = request->getParam("scen")->value().toInt();
            jsonWriteBool(configSetupJson, "scen", value);
            saveConfig();
            Scenario::load();
            request->send(200);
        } else if (request->hasArg("sceninit")) {
            Scenario::load();
            request->send(200);
        } else if (request->hasArg("cleanlog")) {
            clean_log_date();
            request->send(200);
        } else if (request->hasArg("udponoff")) {
            bool value = request->getParam("udponoff")->value().toInt();
            jsonWriteBool(configSetupJson, "udponoff", value);
            saveConfig();
            Scenario::load();
            request->send(200);
        } else if (request->hasArg("updatelist")) {
            removeFile("/dev.csv");
            addFile("dev.csv", "device id;device name;ip address");
            request->redirect("/?set.udp");
        } else if (request->hasArg("updatepage")) {
            request->redirect("/?set.udp");
        } else if (request->hasArg("devname")) {
            jsonWriteStr(configSetupJson, "name", request->getParam("devname")->value());
            saveConfig();
            request->send(200);
        } else if (request->hasArg("routerssid")) {
            jsonWriteStr(configSetupJson, "routerssid", request->getParam("routerssid")->value());
            saveConfig();
            request->send(200);
        } else if (request->hasArg("routerpass")) {
            jsonWriteStr(configSetupJson, "routerpass", request->getParam("routerpass")->value());
            saveConfig();
            request->send(200);
        } else if (request->hasArg("apssid")) {
            jsonWriteStr(configSetupJson, "apssid", request->getParam("apssid")->value());
            saveConfig();
            request->send(200);
        } else if (request->hasArg("appass")) {
            jsonWriteStr(configSetupJson, "appass", request->getParam("appass")->value());
            saveConfig();
            request->send(200);
        } else if (request->hasArg("weblogin")) {
            jsonWriteStr(configSetupJson, "weblogin", request->getParam("weblogin")->value());
            saveConfig();
            request->send(200);
        } else if (request->hasArg("webpass")) {
            jsonWriteStr(configSetupJson, "webpass", request->getParam("webpass")->value());
            saveConfig();
            request->send(200);
        } else if (request->hasArg("timezone")) {
            String timezoneStr = request->getParam("timezone")->value();
            jsonWriteStr(configSetupJson, "timezone", timezoneStr);
            saveConfig();
            timeNow->setTimezone(timezoneStr.toInt());
            request->send(200);
        } else if (request->hasArg("ntp")) {
            String ntpStr = request->getParam("ntp")->value();
            jsonWriteStr(configSetupJson, "ntp", ntpStr);
            saveConfig();
            timeNow->setNtpPool(ntpStr);
            request->send(200);
        } else if (request->hasArg("blink")) {
            bool value = request->getParam("blink")->value().toInt();
            jsonWriteBool(configSetupJson, "blink", value);
            saveConfig();
            request->send(200);
        } else if (request->hasArg("mqttServer")) {
            jsonWriteStr(configSetupJson, "mqttServer", request->getParam("mqttServer")->value());
            saveConfig();
            mqttParamsChanged = true;
            request->send(200);
        } else if (request->hasArg("mqttPort")) {
            int port = (request->getParam("mqttPort")->value()).toInt();
            jsonWriteInt(configSetupJson, "mqttPort", port);
            saveConfig();
            mqttParamsChanged = true;
            request->send(200);
        } else if (request->hasArg("mqttPrefix")) {
            jsonWriteStr(configSetupJson, "mqttPrefix", request->getParam("mqttPrefix")->value());
            saveConfig();
            mqttParamsChanged = true;
            request->send(200);
        } else if (request->hasArg("mqttUser")) {
            jsonWriteStr(configSetupJson, "mqttUser", request->getParam("mqttUser")->value());
            saveConfig();
            mqttParamsChanged = true;
            request->send(200);
        } else if (request->hasArg("mqttPass")) {
            jsonWriteStr(configSetupJson, "mqttPass", request->getParam("mqttPass")->value());
            saveConfig();
            mqttParamsChanged = true;
            request->send(200);
        } else if (request->hasArg("mqttsend")) {
            mqtt_send_settings_to_udp = true;
            request->send(200);
        } else if (request->hasArg("mqttcheck")) {
            String buf = "<button class=\"close\" onclick=\"toggle('my-block')\">×</button>" + MqttClient::getStateStr();
            String payload = "{}";
            jsonWriteStr(payload, "title", buf);
            jsonWriteStr(payload, "class", "pop-up");
            request->send(200, "text/html", payload);
        } else if (request->hasArg("pushingboxid")) {
            jsonWriteStr(configSetupJson, "pushingboxid", request->getParam("pushingboxid")->value());
            saveConfig();
            request->send(200);
        } else if (request->hasArg(TAG_I2C)) {
            busScanFlag = true;
            busToScan = BS_I2C;
            request->redirect("/?set.utilities");
        } else if (request->hasArg(TAG_ONE_WIRE)) {
            busScanFlag = true;
            busToScan = BS_ONE_WIRE;
            if (request->hasParam(TAG_ONE_WIRE_PIN)) {
                setConfigParam(TAG_ONE_WIRE_PIN, request->getParam(TAG_ONE_WIRE_PIN)->value());
            }
            request->redirect("/?set.utilities");
        } else if (request->hasArg(TAG_ONE_WIRE_PIN)) {
            setConfigParam(TAG_ONE_WIRE_PIN, request->getParam(TAG_ONE_WIRE_PIN)->value());
            request->send(200);
        } else {
            request->send(404);
        }
    });

    /* 
    * Check
    */
    server.on("/check", HTTP_GET, [](AsyncWebServerRequest* request) {
        String msg = "";
        // Errors
        if (!FLASH_4MB) {
            msg = F("Обновление \"по воздуху\" не поддерживается!");
        } else if (!isNetworkActive()) {
            msg = F("Устройство не подключено к роутеру!");
        } else if (lastVersion == "error") {
            msg = F("Cервер не найден. Попробуйте повторить позже...");
        } else if (lastVersion == "notsupported") {
            msg = F("Обновление возможно только через usb!");
        } else if (lastVersion.isEmpty()) {
            checkUpdatesFlag = true;
            msg = F("Нажмите на кнопку \"обновить прошивку\" повторно...");
        } else {
            pm.info("version: " + lastVersion);
            // TODO Версия должна быть выше
            if (lastVersion == FIRMWARE_VERSION) {
                msg = F("Актуальная версия прошивки уже установлена.");
            } else if (lastVersion != FIRMWARE_VERSION) {
                msg = F("Доступно обновление<a href=\"#\" class=\"btn btn-block btn-danger\" onclick=\"send_request(this, '/upgrade');setTimeout(function(){ location.href='/'; }, 120000);html('my-block','<span class=loader></span>Идет установка, по ее заверщению страница автоматически перезагрузится...')\">Установить</a>");
            }
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
        updateFlag = true;
        request->send(200, "text/html");
    });
}