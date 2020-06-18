#include "Global.h"

boolean RouterFind(String ssid);

void WIFI_init()
{
    // --------------------Получаем ssid password со страницы
    server.on("/ssid", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (request->hasArg("ssid"))
        {
            jsonWriteStr(configSetup, "ssid", request->getParam("ssid")->value());
        }
        if (request->hasArg("password"))
        {
            jsonWriteStr(configSetup, "password", request->getParam("password")->value());
        }
        saveConfig();                          // Функция сохранения данных во Flash
        request->send(200, "text/text", "OK"); // отправляем ответ о выполнении
    });
    // --------------------Получаем ssidAP passwordAP со страницы
    server.on("/ssidap", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (request->hasArg("ssidAP"))
        {
            jsonWriteStr(configSetup, "ssidAP", request->getParam("ssidAP")->value());
        }
        if (request->hasArg("passwordAP"))
        {
            jsonWriteStr(configSetup, "passwordAP", request->getParam("passwordAP")->value());
        }
        saveConfig();                          // Функция сохранения данных во Flash
        request->send(200, "text/text", "OK"); // отправляем ответ о выполнении
    });

    // --------------------Получаем логин и пароль для web со страницы
    server.on("/web", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (request->hasArg("web_login"))
        {
            jsonWriteStr(configSetup, "web_login", request->getParam("web_login")->value());
        }
        if (request->hasArg("web_pass"))
        {
            jsonWriteStr(configSetup, "web_pass", request->getParam("web_pass")->value());
        }
        saveConfig(); // Функция сохранения данных во Flash
        //Web_server_init();
        request->send(200, "text/text", "OK"); // отправляем ответ о выполнении
    });

    server.on("/restart", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (request->hasArg("device"))
        {
            if (request->getParam("device")->value() == "ok")
                ESP.restart();
        }
        request->send(200, "text/text", "OK"); // отправляем ответ о выполнении
    });
    ROUTER_Connecting();
}

void ROUTER_Connecting()
{
    led_blink("slow");

    WiFi.mode(WIFI_STA);

    byte tries = 20;
    String _ssid = jsonReadStr(configSetup, "ssid");
    String _password = jsonReadStr(configSetup, "password");
    //WiFi.persistent(false);

    if (_ssid == "" && _password == "")
    {
        WiFi.begin();
    }
    else
    {
        WiFi.begin(_ssid.c_str(), _password.c_str());
        Serial.print("ssid: ");
        Serial.println(_ssid);
    }
    // Делаем проверку подключения до тех пор пока счетчик tries
    // не станет равен нулю или не получим подключение
    while (--tries && WiFi.status() != WL_CONNECTED)
    {
        if (WiFi.status() == WL_CONNECT_FAILED)
        {
            Serial.println("[E] password is not correct");
            tries = 1;
            jsonWriteInt(optionJson, "pass_status", 1);
        }
        Serial.print(".");
        delay(1000);
    }

    Serial.println();
    if (WiFi.status() != WL_CONNECTED)
    {
        StartAPMode();
    }
    else
    {
        Serial.println("[V] WiFi connected");
        Serial.print("[V] IP address: http://");
        Serial.print(WiFi.localIP());
        Serial.println();

        jsonWriteStr(configJson, "ip", WiFi.localIP().toString());
        led_blink("off");
    }
}

bool StartAPMode()
{
    Serial.println("WiFi up AP");
    WiFi.disconnect();

    WiFi.mode(WIFI_AP);

    String _ssidAP = jsonReadStr(configSetup, "ssidAP");
    String _passwordAP = jsonReadStr(configSetup, "passwordAP");
    WiFi.softAP(_ssidAP.c_str(), _passwordAP.c_str());
    IPAddress myIP = WiFi.softAPIP();
    led_blink("on");
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    jsonWriteStr(configJson, "ip", myIP.toString());

    if (jsonReadInt(optionJson, "pass_status") != 1)
    {
        ts.add(
            ROUTER_SEARCHING, 10 * 1000, [&](void *) {
                Serial.println("->try find router");
                if (RouterFind(jsonReadStr(configSetup, "ssid")))
                {
                    ts.remove(ROUTER_SEARCHING);
                    WiFi.scanDelete();
                    ROUTER_Connecting();
                    MQTT_init();
                }
            },
            nullptr, true);
    }
    return true;
}

boolean RouterFind(String ssid)
{
    bool res = false;

    int n = WiFi.scanComplete();
    Serial.println("n = " + String(n));
    if (n == -2)
    { //Сканирование не было запущено, запускаем
        Serial.println("[WIFI][i] scanning has not been triggered, starting scanning");
        WiFi.scanNetworks(true, false); //async, show_hidde
    }
    else if (n == -1)
    { //Сканирование все еще выполняется
        Serial.println("[WIFI][i] scanning still in progress");
    }
    else if (n == 0)
    { //ни одна сеть не найдена
        Serial.println("[WIFI][i] no any wifi sations, starting scanning");
        WiFi.scanNetworks(true, false);
    }
    else if (n > 0)
    {
        for (int i = 0; i <= n; i++)
        {
            if (WiFi.SSID(i) == ssid)
            {
                WiFi.scanDelete();
                res = true;
            }
            else
            {
                Serial.print(i);
                Serial.print(")");
                if (i == n)
                {
                    Serial.print(WiFi.SSID(i));
                    Serial.println("; ");
                }
                else
                {
                    Serial.print(WiFi.SSID(i));
                    Serial.println("; ");
                }
            }
        }
        WiFi.scanDelete();
    }
    return res;
}
