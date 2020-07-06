#include <Arduino.h>

#include "Global.h"

static const char* MODULE = "Web";

static const char* TAG_SET_UTILITIES = "/?set.utilities";

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
            if (request->hasArg("check_mqtt")) {
                String buf = "<button class=\"close\" onclick=\"toggle('my-block')\">×</button>" + MqttClient::getStateStr();
                String payload = "{}";
                jsonWriteStr(payload, "title", buf);
                jsonWriteStr(payload, "class", "pop-up");
                request->send(200, "text/html", payload);
                return;
            }
            if (request->hasArg("share_mqtt")) {
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
                pm.info(param + ": " + value);
                if (!config.setParamByName(param, value)) {
                    pm.error("unknown: " + param);
                    request->send(404);
                    return;
                }
            }
            request->send(200);
        });

    server.on("/check", HTTP_GET, [](AsyncWebServerRequest* request) {
        String msg = "";
        String lastVersion = Updater::check();
        // Errors
        if (!FLASH_4MB) {
            msg = F("Обновление \"по воздуху\" не поддерживается!");
        } else if (!NetworkManager::isNetworkActive()) {
            msg = F("Устройство не подключено к роутеру!");
        } else if (lastVersion == "error") {
            msg = F("Cервер не найден. Попробуйте повторить позже...");
        } else if (lastVersion == "notsupported") {
            msg = F("Обновление возможно только через usb!");
        } else if (lastVersion.isEmpty()) {
            perform_updates_check();
            msg = F("Нажмите на кнопку \"обновить прошивку\" повторно...");
        } else {
            // TODO Версия должна быть выше
            if (lastVersion == FIRMWARE_VERSION) {
                msg = F("Актуальная версия прошивки уже установлена.");
            } else if (lastVersion != FIRMWARE_VERSION) {
                msg = F("Доступно обновление");
                msg += lastVersion;
                msg += F("<a href =\"#\" class=\"btn btn-block btn-danger\" onclick=\"send_request(this, '/upgrade');setTimeout(function(){ location.href='/'; }, 120000);html('my-block','<span class=loader></span>Идет установка, по ее заверщению страница автоматически перезагрузится...')\">Установить</a>");
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