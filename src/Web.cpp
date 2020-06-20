#include "Global.h"

String stateMQTT();

void web_init() {
    server.on("/set", HTTP_GET, [](AsyncWebServerRequest* request) {
        String value;
        //============================device settings=====================================
        if (request->hasArg("preset")) {
            //--------------------------------------------------------------------------------
            String value;
            value = request->getParam("preset")->value();
            if (value == "1") {
                writeFile("firmware.c.txt", readFile("configs/1-relay.c.txt", 2048));
                writeFile("firmware.s.txt", readFile("configs/1-relay.s.txt", 2048));
            }
            if (value == "2") {
                writeFile("firmware.c.txt", readFile("configs/2-relay.c.txt", 2048));
                writeFile("firmware.s.txt", readFile("configs/2-relay.s.txt", 2048));
            }
            if (value == "3") {
                writeFile("firmware.c.txt", readFile("configs/3-relay.c.txt", 2048));
                writeFile("firmware.s.txt", readFile("configs/3-relay.s.txt", 2048));
            }
            if (value == "4") {
                writeFile("firmware.c.txt", readFile("configs/4-relay.c.txt", 2048));
                writeFile("firmware.s.txt", readFile("configs/4-relay.s.txt", 2048));
            }
            if (value == "5") {
                writeFile("firmware.c.txt", readFile("configs/5-relay.c.txt", 2048));
                writeFile("firmware.s.txt", readFile("configs/5-relay.s.txt", 2048));
            }
            if (value == "6") {
                writeFile("firmware.c.txt", readFile("configs/6-relay.c.txt", 2048));
                writeFile("firmware.s.txt", readFile("configs/6-relay.s.txt", 2048));
            }
            if (value == "7") {
                writeFile("firmware.c.txt", readFile("configs/7-relay.c.txt", 2048));
                writeFile("firmware.s.txt", readFile("configs/7-relay.s.txt", 2048));
            }
            if (value == "8") {
                writeFile("firmware.c.txt", readFile("configs/8-pwm.c.txt", 2048));
                writeFile("firmware.s.txt", readFile("configs/8-pwm.s.txt", 2048));
            }
            if (value == "9") {
                writeFile("firmware.c.txt", readFile("configs/9-dht11.c.txt", 2048));
                writeFile("firmware.s.txt", readFile("configs/9-dht11.s.txt", 2048));
            }
            if (value == "10") {
                writeFile("firmware.c.txt", readFile("configs/10-dht22.c.txt", 2048));
                writeFile("firmware.s.txt", readFile("configs/10-dht22.s.txt", 2048));
            }
            if (value == "11") {
                writeFile("firmware.c.txt", readFile("configs/11-analog.c.txt", 2048));
                writeFile("firmware.s.txt", readFile("configs/11-analog.s.txt", 2048));
            }
            if (value == "12") {
                writeFile("firmware.c.txt", readFile("configs/12-bmp280.c.txt", 2048));
                writeFile("firmware.s.txt", readFile("configs/12-bmp280.s.txt", 2048));
            }
            if (value == "13") {
                writeFile("firmware.c.txt", readFile("configs/13-bme280.c.txt", 2048));
                writeFile("firmware.s.txt", readFile("configs/13-bme280.s.txt", 2048));
            }
            if (value == "14") {
                writeFile("firmware.c.txt", readFile("configs/14-dallas.c.txt", 2048));
                writeFile("firmware.s.txt", readFile("configs/14-dallas.s.txt", 2048));
            }
            if (value == "15") {
                writeFile("firmware.c.txt", readFile("configs/15-termostat.c.txt", 2048));
                writeFile("firmware.s.txt", readFile("configs/15-termostat.s.txt", 2048));
            }
            if (value == "16") {
                writeFile("firmware.c.txt", readFile("configs/16-ultrasonic.c.txt", 2048));
                writeFile("firmware.s.txt", readFile("configs/16-ultrasonic.s.txt", 2048));
            }
            if (value == "17") {
                writeFile("firmware.c.txt", readFile("configs/17-moution.c.txt", 2048));
                writeFile("firmware.s.txt", readFile("configs/17-moution.s.txt", 2048));
            }
            if (value == "18") {
                writeFile("firmware.c.txt", readFile("configs/18-moution.c.txt", 2048));
                writeFile("firmware.s.txt", readFile("configs/18-moution.s.txt", 2048));
            }
            if (value == "19") {
                writeFile("firmware.c.txt", readFile("configs/19-stepper.c.txt", 2048));
                writeFile("firmware.s.txt", readFile("configs/19-stepper.s.txt", 2048));
            }
            if (value == "20") {
                writeFile("firmware.c.txt", readFile("configs/20-servo.c.txt", 2048));
                writeFile("firmware.s.txt", readFile("configs/20-servo.s.txt", 2048));
            }
            if (value == "21") {
                writeFile("firmware.c.txt", readFile("configs/firmware.c.txt", 2048));
                writeFile("firmware.s.txt", readFile("configs/firmware.s.txt", 2048));
            }
            Device_init();
            Scenario_init();
            request->redirect("/?set.device");
        }
        //--------------------------------------------------------------------------------
        if (request->hasArg("devinit")) {
            Device_init();
            request->send(200, "text/text", "OK");
        }
        //--------------------------------------------------------------------------------
        if (request->hasArg("scen")) {
            value = request->getParam("scen")->value();
            if (value == "0") {
                jsonWriteStr(configSetupJson, "scen", value);
                saveConfig();
                Scenario_init();
            }
            if (value == "1") {
                jsonWriteStr(configSetupJson, "scen", value);
                saveConfig();
                Scenario_init();
            }
            request->send(200, "text/text", "OK");
        }
        //--------------------------------------------------------------------------------
        if (request->hasArg("sceninit")) {
            Scenario_init();
            request->send(200, "text/text", "OK");
        }
        //--------------------------------------------------------------------------------
#ifdef logging_enable
        if (request->hasArg("cleanlog")) {
            clean_log_date();
            request->send(200, "text/text", "OK");
        }
#endif
        //==============================udp settings=============================================
        if (request->hasArg("udponoff")) {
            value = request->getParam("udponoff")->value();
            if (value == "0") {
                jsonWriteStr(configSetupJson, "udponoff", value);
                saveConfig();
                Scenario_init();
            }
            if (value == "1") {
                jsonWriteStr(configSetupJson, "udponoff", value);
                saveConfig();
                Scenario_init();
            }
            request->send(200, "text/text", "OK");
        }
        //--------------------------------------------------------------------------------
        if (request->hasArg("updatelist")) {
            LittleFS.remove("/dev.csv");
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
            jsonWriteStr(configSetupJson, "timezone", request->getParam("timezone")->value());
            saveConfig();
            reconfigTime();
            request->send(200, "text/text", "OK");
        }
        if (request->hasArg("ntp")) {
            jsonWriteStr(configSetupJson, "ntp", request->getParam("ntp")->value());
            saveConfig();
            reconfigTime();
            request->send(200, "text/text", "OK");
        }
        //--------------------------------------------------------------------------------
        if (request->hasArg("device")) {
            if (request->getParam("device")->value() == "ok") ESP.restart();
            request->send(200, "text/text", "OK");
        }
        //--------------------------------------------------------------------------------
        if (request->hasArg("blink")) {
            value = request->getParam("blink")->value();
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
            mqtt_connection = true;
            request->send(200, "text/text", "ok");
        }
        if (request->hasArg("mqttPort")) {
            int port = (request->getParam("mqttPort")->value()).toInt();
            jsonWriteInt(configSetupJson, "mqttPort", port);
            saveConfig();
            mqtt_connection = true;
            request->send(200, "text/text", "ok");
        }
        if (request->hasArg("mqttPrefix")) {
            jsonWriteStr(configSetupJson, "mqttPrefix", request->getParam("mqttPrefix")->value());
            saveConfig();
            mqtt_connection = true;
            request->send(200, "text/text", "ok");
        }
        if (request->hasArg("mqttUser")) {
            jsonWriteStr(configSetupJson, "mqttUser", request->getParam("mqttUser")->value());
            saveConfig();
            mqtt_connection = true;
            request->send(200, "text/text", "ok");
        }
        if (request->hasArg("mqttPass")) {
            jsonWriteStr(configSetupJson, "mqttPass", request->getParam("mqttPass")->value());
            saveConfig();
            mqtt_connection = true;
            request->send(200, "text/text", "ok");
        }
        //--------------------------------------------------------------------------------
        if (request->hasArg("mqttsend")) {
            mqtt_send_settings_to_udp = true;
            request->send(200, "text/text", "ok");
        }
        //--------------------------------------------------------------------------------
        if (request->hasArg("mqttcheck")) {
            String tmp = "{}";
            jsonWriteStr(tmp, "title", "<button class=\"close\" onclick=\"toggle('my-block')\">×</button>" + stateMQTT());
            jsonWriteStr(tmp, "class", "pop-up");
            request->send(200, "text/text", tmp);
        }
        //==============================push settings=============================================
#ifdef push_enable
        if (request->hasArg("pushingboxid")) {
            jsonWriteStr(configSetupJson, "pushingboxid", request->getParam("pushingboxid")->value());
            saveConfig();
            request->send(200, "text/text", "ok");
        }
#endif
        //==============================utilities settings=============================================
        if (request->hasArg("itoc")) {
            i2c_scanning = true;
            request->redirect("/?set.utilities");
        }
    });
    //==============================upgrade settings=============================================
    server.on("/check", HTTP_GET, [](AsyncWebServerRequest* request) {
        upgrade_url = true;
        Serial.print("[I] Last firmware version: ");
        Serial.println(last_version);
        String tmp = "{}";
        int case_of_update;

        if (WiFi.status() != WL_CONNECTED) last_version = "nowifi";
        if (!mb_4_of_memory) last_version = "less";

        if (last_version == firmware_version) case_of_update = 1;
        if (last_version != firmware_version) case_of_update = 2;
        if (last_version == "error") case_of_update = 3;
        if (last_version == "") case_of_update = 4;
        if (last_version == "less") case_of_update = 5;
        if (last_version == "nowifi") case_of_update = 6;
        if (last_version == "notsupported") case_of_update = 7;

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