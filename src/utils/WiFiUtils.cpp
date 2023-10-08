#include "utils/WiFiUtils.h"
#include <vector>
#define TRIESONE 25 // количество попыток подключения к одной сети из несколких
#define TRIES 40    // количество попыток подключения сети если она одна

wifi_t g_WiFi;

void WiFiInit()
{
  g_WiFi.state_scan = _FindAP_end;
  g_WiFi.mode_scan_for_ws = false;
  g_WiFi.mode_scan_for_connect = false;
}

void routerConnect()
{
  WiFi.setAutoConnect(false);
  WiFi.persistent(false);

  WiFi.mode(WIFI_STA);
  byte triesOne = TRIESONE;

  std::vector<String> _ssidList;
  std::vector<String> _passwordList;
  jsonReadArray(settingsFlashJson, "routerssid", _ssidList);
  jsonReadArray(settingsFlashJson, "routerpass", _passwordList);
  if (_ssidList.size() > 1)
    triesOne = TRIES;

  if (_passwordList.size() == 0 && _ssidList[0] == "" && _passwordList[0] == "")
  {
    WiFi.begin();
  }
  else
  {
    WiFi.begin(_ssidList[0].c_str(), _passwordList[0].c_str());
#ifdef ESP32
    WiFi.setTxPower(WIFI_POWER_19_5dBm);
#else
    WiFi.setOutputPower(20.5);
#endif
    String _ssid;
    String _password;
    for (int8_t i = 0; i < _ssidList.size(); i++)
    {
      _ssid = _ssid + _ssidList[i] + "; ";
    }
    for (int8_t i = 0; i < _passwordList.size(); i++)
    {
      _password = _password + _passwordList[i] + "; ";
    }
    SerialPrint("i", "WIFI", "ssid list: " + _ssid);
    SerialPrint("i", "WIFI", "pass list: " + _password);
  }
  for (size_t i = 0; i < _ssidList.size(); i++)
  {
    triesOne = TRIESONE;
    if (isNetworkActive())
      break;
    WiFi.begin(_ssidList[i].c_str(), _passwordList[i].c_str());
    SerialPrint("i", "WIFI", "ssid connect: " + _ssidList[i]);
    SerialPrint("i", "WIFI", "pass connect: " + _passwordList[i]);
    while (--triesOne && !isNetworkActive())
    {
//            SerialPrint("i", "WIFI", ": " + String((int)WiFi.status()));
#ifdef ESP8266
      if (WiFi.status() == WL_CONNECT_FAILED || WiFi.status() == WL_WRONG_PASSWORD)
#else
      if (WiFi.status() == WL_CONNECT_FAILED)
#endif
      {
        SerialPrint("E", "WIFI", "password is not correct");
        triesOne = 1;
        jsonWriteInt(errorsHeapJson, "passer", 1);
        break;
      }
      Serial.print(".");
      delay(1000);
    }
    Serial.println("");
  }

  if (!isNetworkActive())
  {
    Serial.println("");
    startAPMode();
  }
  else
  {
    Serial.println("");
    SerialPrint("i", "WIFI", "http://" + WiFi.localIP().toString());
    jsonWriteStr(settingsFlashJson, "ip", WiFi.localIP().toString());

    mqttInit();
  }
  SerialPrint("i", F("WIFI"), F("Network Init"));
}

bool startAPMode()
{
  SerialPrint("i", "WIFI", "AP Mode");

  WiFi.disconnect();
  WiFi.mode(WIFI_AP);

  String _ssidAP = jsonReadStr(settingsFlashJson, "apssid");
  String _passwordAP = jsonReadStr(settingsFlashJson, "appass");

  WiFi.softAP(_ssidAP.c_str(), _passwordAP.c_str());
  IPAddress myIP = WiFi.softAPIP();

  SerialPrint("i", "WIFI", "AP IP: " + myIP.toString());
  jsonWriteStr(settingsFlashJson, "ip", myIP.toString());
  AsyncRouterFind(_scan_for_connect);
  return true;
}

void AsyncRouterFind(mode_scan_t mode)
{
  switch (mode) {
  case _scan_for_ws:
    g_WiFi.mode_scan_for_ws = true;
    break;
  case _scan_for_connect:
    g_WiFi.mode_scan_for_connect = true;
    break;
  default:
    break;
  }
  if(g_WiFi.state_scan == _FindAP_end) {
    ts.add(WIFI_SCAN, 1000, [&](void *) {
      std::vector<String> jArray;
      jsonReadArray(settingsFlashJson, "routerssid", jArray);
      switch (g_WiFi.state_scan) {
        default:
        case _FindAP_start:
        {
          WiFi.scanNetworks(true, true);
          SerialPrint("i", F("WIFI_scan"), "Start");
          for (int8_t i = 0; i < jArray.size(); i++)
            SerialPrint("i", F("WIFI_scan"), "scanning for " + jArray[i]);
          g_WiFi.state_scan = _FindAP_finding;
          break;
        }
        case _FindAP_finding:
        {
          int8_t n = WiFi.scanComplete();
          if (n == -1) // еще ищем
              break;
          else if (n == -2) {
              SerialPrint("E", F("WIFI_scan"), "Error");
              g_WiFi.state_scan = _FindAP_start;
              // g_WiFi.StateFind = _FindAP_end;
              // break;
          } else if (n == 0) {
              SerialPrint("i", F("WIFI_scan"), "No WiFi");
              // ssidListHeapJson = "{}";
              g_WiFi.state_scan = _FindAP_start;
              // g_WiFi.StateFind = _FindAP_end;
              // break;
          } else if (n > 0) {
            bool ssid_available = false;
            ssidListHeapJson = "{}"; // очищаем, ранее были задвоения сетей
            String current_SSID = "";
            if(isNetworkActive()) {
              current_SSID = WiFi.SSID();
            }
            for (int8_t i = 0; i < n; i++)
            {
              // String i_SSID = WiFi.SSID(i);
              for (int8_t k = 0; k < jArray.size(); k++) {
                if (WiFi.SSID(i) == jArray[k]) {
                  ssid_available = true;
                }
              }
              // SerialPrint("i", "WIFI", (res ? "*" : "") + String(i, DEC) + ") " + WiFi.SSID(i));
              if (WiFi.SSID(i) != "")
                jsonWriteStr_(ssidListHeapJson, String(i+1), WiFi.SSID(i));
            }
            WiFi.scanDelete();
            SerialPrint("i", "WIFI_scan", ssidListHeapJson);
            jsonWriteStr_(ssidListHeapJson, "0", jsonReadStr(settingsFlashJson, F("routerssid"))); // чтобы всегда была первой в списке
            if(g_WiFi.mode_scan_for_ws) {
              sendStringToWs("ssidli", ssidListHeapJson, -1);
              g_WiFi.mode_scan_for_ws = false;
            }
            if(g_WiFi.mode_scan_for_connect) {
              if(ssid_available) {
                if(!isNetworkActive()) {
                  routerConnect();
                }
              } else {
                g_WiFi.state_scan = _FindAP_start; // бесконечно ищем наши сети
                break;
              }
              g_WiFi.mode_scan_for_connect = false;
            }
          }
          g_WiFi.state_scan = _FindAP_end;
          // break;
      // case _FindAP_end:
          SerialPrint("i", F("WIFI_scan"), "End");
          ts.remove(WIFI_SCAN);
          break;
        }
      }
    }, nullptr, true);
  }
}

boolean isNetworkActive() {
    return WiFi.status() == WL_CONNECTED;
}

uint8_t getNumAPClients() {
    return WiFi.softAPgetStationNum();
}

uint8_t RSSIquality() {
    uint8_t res = 0;
    if (isNetworkActive()) {
        int rssi = WiFi.RSSI();
        if (rssi >= -50) {
            res = 6;  //"Excellent";
        } else if (rssi < -50 && rssi >= -60) {
            res = 5;  //"Very good";
        } else if (rssi < -60 && rssi >= -70) {
            res = 4;  //"Good";
        } else if (rssi < -70 && rssi >= -80) {
            res = 3;  //"Low";
        } else if (rssi < -80 && rssi > -100) {
            res = 2;  //"Very low";
        } else if (rssi <= -100) {
            res = 1;  //"No signal";
        }
    }
    return res;
}
