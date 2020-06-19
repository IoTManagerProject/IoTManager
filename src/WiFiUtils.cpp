#include "Global.h"

boolean RouterFind(String ssid);

void ROUTER_Connecting() {
  
  led_blink("slow");

  WiFi.mode(WIFI_STA);

  byte tries = 20;
  String _ssid = jsonReadStr(configSetupJson, "routerssid");
  String _password = jsonReadStr(configSetupJson, "routerpass");
  //WiFi.persistent(false);

  if (_ssid == "" && _password == "") {
    WiFi.begin();
  }
  else {
    WiFi.begin(_ssid.c_str(), _password.c_str());
    Serial.print("ssid: ");
    Serial.println(_ssid);
  }
  // Делаем проверку подключения до тех пор пока счетчик tries
  // не станет равен нулю или не получим подключение
  while (--tries && WiFi.status() != WL_CONNECTED) {
    if (WiFi.status() == WL_CONNECT_FAILED) {
      Serial.println("[E] password is not correct");
      tries = 1;
      jsonWriteInt(configOptionJson, "pass_status", 1);
    }
    Serial.print(".");
    delay(1000);
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    // Если не удалось подключиться запускаем в режиме AP
    Serial.println("");
    // WiFi.disconnect(true);
    StartAPMode();

  }
  else {
    // Иначе удалось подключиться отправляем сообщение
    // о подключении и выводим адрес IP
    Serial.println("");
    Serial.println("[V] WiFi connected");
    Serial.print("[V] IP address: http://");
    Serial.print(WiFi.localIP());
    Serial.println("");
    jsonWriteStr(configSetupJson, "ip", WiFi.localIP().toString());
    led_blink("off");
    //add_dev_in_list("dev.txt", chipID, WiFi.localIP().toString());
    MQTT_init();
  }
}

bool StartAPMode() {
  Serial.println("WiFi up AP");
  WiFi.disconnect();

  WiFi.mode(WIFI_AP);

  String _ssidAP = jsonReadStr(configSetupJson, "apssid");
  String _passwordAP = jsonReadStr(configSetupJson, "appass");
  WiFi.softAP(_ssidAP.c_str(), _passwordAP.c_str());
  IPAddress myIP = WiFi.softAPIP();
  led_blink("on");
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  jsonWriteStr(configSetupJson, "ip", myIP.toString());

  //if (jsonReadInt(configOptionJson, "pass_status") != 1) {
    ts.add(ROUTER_SEARCHING, 10 * 1000, [&](void*) {
      Serial.println("->try find router");
      if (RouterFind(jsonReadStr(configSetupJson, "routerssid"))) {
        ts.remove(ROUTER_SEARCHING);
        WiFi.scanDelete();
        ROUTER_Connecting();
      }
    }, nullptr, true);
  //}
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