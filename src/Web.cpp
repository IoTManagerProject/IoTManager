#include <Arduino.h>

#include "Global.h"

static const char* MODULE = "Web";
static const char* PAGE_UTILITIES = "/?set.utilities";
static const char* PAGE_SETUP = "/?set.device";

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
        "/config", HTTP_GET, [](AsyncWebServerRequest* request) {
            if (request->hasArg("add")) {
                configAdd(request->getParam("add")->value());
                request->redirect(PAGE_SETUP);
                return;
            }
        });

    server.on(
        "/set", HTTP_GET, [](AsyncWebServerRequest* request) {
            if (request->hasArg("preset")) {
                setPreset(request->getParam("preset")->value().toInt());
                request->redirect(PAGE_SETUP);
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
            if (request->hasArg("clear_log")) {
                perform_logger_clear();
                request->redirect(PAGE_UTILITIES);
                return;
            }
            if (request->hasArg(TAG_CHECK_MQTT)) {
                DynamicJsonBuffer json;
                JsonObject& root = json.createObject();
                root["title"] = "<button class=\"close\" onclick=\"toggle('my-block')\">×</button>" + MqttClient::getStateStr();
                root["class"] = "pop-up";
                String buf;
                root.printTo(buf);
                request->send(200, "text/html", buf);
                return;
            }
            if (request->hasArg(TAG_SHARE_MQTT)) {
                broadcast_mqtt_settings();
                request->send(200);
                return;
            }
            if (request->hasArg(TAG_I2C)) {
                perform_bus_scanning(BS_I2C);
                request->redirect(PAGE_UTILITIES);
                return;
            }
            if (request->hasArg(TAG_ONE_WIRE)) {
                perform_bus_scanning(BS_ONE_WIRE);
                request->redirect(PAGE_UTILITIES);
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
        String lastVersion = runtime.read(TAG_LAST_VERSION);
        // Errors
        if (!FLASH_4MB) {
            msg = F("Обновление \"по воздуху\" не поддерживается!");
        } else if (!NetworkManager::isNetworkActive()) {
            msg = F("Устройство не подключено к роутеру!");
        } else if (lastVersion == "notsupported") {
            msg = F("Обновление возможно только через usb!");
        } else if (lastVersion == "error" || lastVersion.isEmpty()) {
            perform_updates_check();
            msg = F("Нажмите на кнопку \"обновить прошивку\" повторно...");
        } else {
            // TODO Версия должна быть выше
            if (lastVersion == FIRMWARE_VERSION) {
                msg = F("Версия прошивки актуальна.");
            } else if (lastVersion != FIRMWARE_VERSION) {
                msg = "Обновление: ";
                msg += lastVersion;
                msg += F("<a href =\"#\" class=\"btn btn-block btn-danger\" onclick=\"send_request(this, '/upgrade');setTimeout(function(){ location.href='/'; }, 120000);html('my-block','<span class=loader></span>Идет установка, по ее заверщению страница автоматически перезагрузится...')\">Установить</a>");
            }
        }
        DynamicJsonBuffer json;
        JsonObject& root = json.createObject();
        root["title"] = "<button class=\"close\" onclick=\"toggle('my-block')\">" + msg + "</button>";
        root["class"] = "pop-up";
        String buf;
        root.printTo(buf);
        request->send(200, "text/html", buf);
    });

    /* 
    * Upgrade
    */
    server.on("/upgrade", HTTP_GET, [](AsyncWebServerRequest* request) {
        perform_upgrade();
        request->send(200);
    });
}