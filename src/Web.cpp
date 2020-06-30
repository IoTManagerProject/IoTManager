#include "Global.h"

#include "MqttClient.h"

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
            copyFile(getConfigFile(preset, CT_CONFIG), DEVICE_CONFIG_FILE);
            copyFile(getConfigFile(preset, CT_SCENARIO), DEVICE_SCENARIO_FILE);
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
        } else if (request->hasArg("mqttcheck")) {
            String buf = "<button class=\"close\" onclick=\"toggle('my-block')\">×</button>" + MqttClient::getStateStr();
            String payload = "{}";
            jsonWriteStr(payload, "title", buf);
            jsonWriteStr(payload, "class", "pop-up");
            request->send(200, "text/html", payload);
            return;
        } else
            for (size_t i = 0; i < request->params(); i++) {
                String param_name = request->getParam(i)->name();
                String param_value = request->getParam(i)->value();
                pm.info(param_name + ":" + param_value);

                if (param_name.equals("routerssid")) {
                    config.network()->setSSID(WIFI_STA, param_value);
                } else if (param_name.equals("routerpass")) {
                    config.network()->setPasswd(WIFI_STA, param_value);
                } else if (param_name.equals("apssid")) {
                    config.network()->setSSID(WIFI_AP, param_value);
                } else if (param_name.equals("appass")) {
                    config.network()->setPasswd(WIFI_AP, param_value);
                } else if (param_name.equals("timezone")) {
                    config.clock()->setTimezone(param_value.toInt());
                } else if (param_name.equals("ntp")) {
                    config.clock()->setNtp(param_value);
                } else if (request->hasArg("weblogin")) {
                    jsonWriteStr(configSetupJson, "weblogin", request->getParam("weblogin")->value());
                    saveConfig();
                } else if (request->hasArg("webpass")) {
                    jsonWriteStr(configSetupJson, "webpass", request->getParam("webpass")->value());
                    saveConfig();
                } else if (request->hasArg("blink")) {
                    jsonWriteBool(configSetupJson, "blink", param_value.toInt());
                    saveConfig();
                } else if (param_name.equals("mqttServer")) {
                    config.mqtt()->setServer(param_value);
                } else if (param_name.equals("mqttPort")) {
                    config.mqtt()->setPort(param_value.toInt());
                } else if (request->hasArg("mqttPrefix")) {
                    config.mqtt()->setPrefix(param_value);
                } else if (request->hasArg("mqttUser")) {
                    config.mqtt()->setUser(param_value);
                } else if (request->hasArg("mqttPass")) {
                    config.mqtt()->setPass(param_value);
                } else if (request->hasArg("mqttsend")) {
                    broadcast_mqtt_settings = true;
                } else if (request->hasArg("pushingboxid")) {
                    jsonWriteStr(configSetupJson, "pushingboxid", request->getParam("pushingboxid")->value());
                    saveConfig();
                    request->send(200);
                    return;
                } else if (request->hasArg(TAG_I2C)) {
                    perform_bus_scanning = true;
                    bus_to_scan = BS_I2C;
                    request->redirect("/?set.utilities");
                    return;
                } else if (request->hasArg(TAG_ONE_WIRE)) {
                    perform_bus_scanning = true;
                    bus_to_scan = BS_ONE_WIRE;
                    if (request->hasParam(TAG_ONE_WIRE_PIN)) {
                        setConfigParam(TAG_ONE_WIRE_PIN, request->getParam(TAG_ONE_WIRE_PIN)->value());
                    }
                    request->redirect("/?set.utilities");
                    return;
                } else if (request->hasArg(TAG_ONE_WIRE_PIN)) {
                    setConfigParam(TAG_ONE_WIRE_PIN, request->getParam(TAG_ONE_WIRE_PIN)->value());
                } else {
                    pm.error("unknown param: " + param_name);
                };
                request->send(200);
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
            perform_updates_check = true;
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
        perform_upgrade = true;
        request->send(200);
    });
}