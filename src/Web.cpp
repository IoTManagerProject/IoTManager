#include "Global.h"

#include "MqttClient.h"

static const char* MODULE = "Web";

static const char* TAG_SET_UTILITIES = "/?set.utilities";

bool configChangeRequest(const String& param, const String& value) {
    pm.info(param + ":" + value);

    if (param.equals("devname")) {
        config.network()->setHostname(value);
    } else if (param.equals("udponoff")) {
        config.general()->enableBroadcast(param.toInt());
    } else if (param.equals("scen")) {
        config.general()->enableScenario(value.toInt());
    } else if (param.equals("routerssid")) {
        config.network()->setSSID(WIFI_STA, value);
    } else if (param.equals("routerpass")) {
        config.network()->setPasswd(WIFI_STA, value);
    } else if (param.equals("apssid")) {
        config.network()->setSSID(WIFI_AP, value);
    } else if (param.equals("appass")) {
        config.network()->setPasswd(WIFI_AP, value);
    } else if (param.equals("timezone")) {
        config.clock()->setTimezone(value.toInt());
    } else if (param.equals("ntp")) {
        config.clock()->setNtp(value);
    } else if (param.equals("weblogin")) {
        config.web()->setLogin(value);
    } else if (param.equals("webpass")) {
        config.web()->setPass(value);
    } else if (param.equals("blink")) {
        config.general()->enableLed(value.toInt());
    } else if (param.equals("mqttServer")) {
        config.mqtt()->setServer(value);
    } else if (param.equals("mqttPort")) {
        config.mqtt()->setPort(value.toInt());
    } else if (param.equals("mqttPrefix")) {
        config.mqtt()->setPrefix(value);
    } else if (param.equals("mqttUser")) {
        config.mqtt()->setUser(value);
    } else if (param.equals("mqttPass")) {
        config.mqtt()->setPass(value);
    } else if (param.equals("pushingboxid")) {
        config.general()->setPushingboxId(value);
    } else {
        return false;
    };
    return true;
};

void web_init() {
    // dnsServer.start(53, "*", WiFi.softAPIP());
    // server.addHandler(new CaptiveRequestHandler(jsonReadStr(configSetupJson, "name").c_str())).setFilter(ON_AP_FILTER);

    server.on("/restart", HTTP_GET, [](AsyncWebServerRequest* request) {
        if (request->hasArg("device")) {
            if (request->getParam("device")->value() == "ok") {
                perform_system_restart();
                request->send(200);
            }
        };
    });

    server.on(
        "/set", HTTP_GET, [](AsyncWebServerRequest* request) {
            if (request->hasArg("preset")) {
                setPreset(request->getParam("preset")->value().toInt());
                request->redirect("/?set.device");
                return;
            }
            if (request->hasArg("devinit")) {
                device_init();
                request->send(200);
                return;
            }
            if (request->hasArg("sceninit")) {
                Scenario::reinit();
                request->send(200);
                return;
            }
            if (request->hasArg("cleanlog")) {
                perform_logger_clear();
                request->redirect(TAG_SET_UTILITIES);
                return;
            }
            if (request->hasArg("updatelist")) {
                removeFile("/dev.csv");
                addFile("dev.csv", "device id;device name;ip address");
                request->redirect("/?set.udp");
                return;
            }
            if (request->hasArg("updatepage")) {
                request->redirect("/?set.udp");
                return;
            }
            if (request->hasArg("mqttcheck")) {
                String buf = "<button class=\"close\" onclick=\"toggle('my-block')\">×</button>" + MqttClient::getStateStr();
                String payload = "{}";
                jsonWriteStr(payload, "title", buf);
                jsonWriteStr(payload, "class", "pop-up");
                request->send(200, "text/html", payload);
                return;
            }
            if (request->hasArg("mqttsend")) {
                broadcast_mqtt_settings();
                request->send(200);
                return;
            }
            if (request->hasArg(TAG_I2C)) {
                perform_bus_scanning(BS_I2C);
                request->redirect(TAG_SET_UTILITIES);
                return;
            }
            if (request->hasArg(TAG_ONE_WIRE)) {
                perform_bus_scanning(BS_ONE_WIRE);
                if (request->hasParam(TAG_ONE_WIRE_PIN)) {
                    runtime.write(TAG_ONE_WIRE_PIN, request->getParam(TAG_ONE_WIRE_PIN)->value().c_str());
                }
                request->redirect(TAG_SET_UTILITIES);
                return;
            }

            for (size_t i = 0; i < request->params(); i++) {
                String param = request->getParam(i)->name();
                String value = request->getParam(i)->value();
                if (!configChangeRequest(param, value)) {
                    pm.error("unknown: " + param);
                    request->send(404);
                    return;
                }
            }
            request->send(200);
        });

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
            perform_updates_check_flag = true;
            msg = F("Нажмите на кнопку \"обновить прошивку\" повторно...");
        } else {
            // TODO Версия должна быть выше
            if (lastVersion == FIRMWARE_VERSION) {
                msg = F("Актуальная версия прошивки уже установлена.");
            } else if (lastVersion != FIRMWARE_VERSION) {
                msg = F("Доступно обновление<a href=\"#\" class=\"btn btn-block btn-danger\" onclick=\"send_request(this, '/upgrade');setTimeout(function(){ location.href='/'; }, 120000);html('my-block','<span class=loader></span>Идет установка, по ее заверщению страница автоматически перезагрузится...')\">Установить</a>");
            }
        }
        String payload = "{}";
        jsonWriteStr(payload, "title", "<button class=\"close\" onclick=\"toggle('my-block')\">×</button>" + msg);
        jsonWriteStr(payload, "class", "pop-up");
        request->send(200, "text/html", payload);
    });

    /* 
    * Upgrade
    */
    server.on("/upgrade", HTTP_GET, [](AsyncWebServerRequest* request) {
        perform_upgrade_flag = true;
        request->send(200);
    });
}