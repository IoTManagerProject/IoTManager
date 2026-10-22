#include "utils/WiFiUtils.h"
#include <vector>
#if defined(ESP32)
#include <esp_task_wdt.h>
#endif
#include "DebugTrace.h"
#define TRIESONE 20 // количество секунд ожидания подключения к одной сети из несколких
#define TRIES 30    // количество секунд ожидания подключения сети если она одна

#if defined(esp32_wifirep)
#include "lwip/lwip_napt.h"
// #include "lwip/ip_route.h"
#define PROTO_TCP 6
#define PROTO_UDP 17

IPAddress stringToIp(String strIp)
{
  IPAddress ip;
  ip.fromString(strIp);
  return ip;
}
#endif
void addPortMap(String TCP_UDP, String maddr, uint16_t mport, String daddr, uint16_t dport)
{
#if defined(esp32_wifirep)
  uint8_t tcp_udp;
  if (TCP_UDP == "TCP")
    tcp_udp = PROTO_TCP;
  else if (TCP_UDP == "UDP")
    tcp_udp = PROTO_UDP;
  else
    SerialPrint("E", "WIFI", "Add port map: ERROR, Must be 'TCP' or 'UDP'");

  ip_portmap_add(tcp_udp, stringToIp(maddr), mport, stringToIp(daddr), dport);
  SerialPrint("i", "WIFI", "Add port map: " + String(tcp_udp) + ", " + maddr + ":" + String(mport) + " -> " + daddr + ":" + String(dport));
#else
  SerialPrint("E", "WIFI", "Add port map: ERROR, change board to esp32_wifirep");
#endif
}



#ifdef WIFI_ASYNC
std::vector<String> _ssidList;
std::vector<String> _passwordList;
// номер сети, для перебирания в момент подключения к сетям из массива
volatile uint8_t currentNetwork = 0;
volatile bool wifiConnecting = false;
volatile uint8_t connectionAttempts = 0;
//------------------------------------------
// Обработчики событий Wi-Fi
//------------------------------------------
void WiFiEvent(arduino_event_t *event)
{
  switch (event->event_id)
  {
#if defined(LIBRETINY) || defined(esp32c6_4mb) || defined(esp32c6_8mb)
  case ARDUINO_EVENT_WIFI_STA_CONNECTED:
#else
  case SYSTEM_EVENT_STA_CONNECTED:
#endif
    // Подключились к STA
    SerialPrint("i", "WIFI", "Connected to AP: " + WiFi.SSID());
    // TODO если подключились, но не получили IP что будет?
    break;
#if defined(LIBRETINY) || defined(esp32c6_4mb) || defined(esp32c6_8mb)
  case ARDUINO_EVENT_WIFI_STA_GOT_IP:
#else
  case SYSTEM_EVENT_STA_GOT_IP:
#endif
    // Получили IP от роутера
    // wifiReconnectTicker.detach();
    ts.remove(WIFI_SCAN);
    ts.remove(WIFI_CONN);
#ifdef LIBRETINY
    SerialPrint("i", "WIFI", "http://" + ipToString(WiFi.localIP()));
    jsonWriteStr(settingsFlashJson, "ip", ipToString(WiFi.localIP()));
#else
    SerialPrint("i", "WIFI", "http://" + WiFi.localIP().toString());
    jsonWriteStr(settingsFlashJson, "ip", WiFi.localIP().toString());
#endif
    createItemFromNet("onWifi", "1", 1);
    // запускаем MQTT
    mqttInit();
    SerialPrint("i", F("WIFI"), F("Network Init"));

    bool postMsgTelegram;
    if (!jsonRead(settingsFlashJson, "debugTraceMsgTlgrm", postMsgTelegram, false)) postMsgTelegram = 1;
    sendDebugTraceAndFreeMemory(postMsgTelegram);

    // Отключаем AP при успешном подключении
    WiFi.softAPdisconnect(true);
    break;
#if defined(LIBRETINY) || defined(esp32c6_4mb) || defined(esp32c6_8mb)
  case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
#else
  case SYSTEM_EVENT_STA_DISCONNECTED:
#endif
    // Отключились от STA
    SerialPrint("i", "WIFI", "Disconnected from STA");
    // Завершаем задачу проверки сети
    ts.remove(WIFI_CONN);
    if (wifiConnecting)
    { // если у нас ещё не закончились попытки подключения, то перезапускаем задачу
      Serial.print(".");
      checkConnection();
      // wifiReconnectTicker.once_ms(WIFI_CHECK_INTERVAL, checkConnection);
    }
    else
    { // если попытки подключения исчерпаны, то переходим в AP
      sendDebugTraceAndFreeMemory(false);
      startAPMode();
    }
    break;
#if defined(LIBRETINY) || defined(esp32c6_4mb) || defined(esp32c6_8mb)
  case ARDUINO_EVENT_WIFI_SCAN_DONE:
#else
  case SYSTEM_EVENT_SCAN_DONE:
#endif
    if (WiFi.scanComplete() >= 0) {
      Serial.println("Valid Scan completed");
      handleScanResults();
      WiFi.scanDelete(); // Очищаем только при успешном сканировании
    } else {
      //Serial.println("Empty scan or error");
      //WiFi.scanDelete(); // Принудительная очистка буфера
    }
    // Завершилось сканирование сетей
    //Serial.println("Scan completed");
    //handleScanResults();
    break;
  }
}

//------------------------------------------
// Обработка результатов сканирования
//------------------------------------------
void handleScanResults()
{
  ssidListHeapJson = "{}";
  _ssidList.clear();
  _passwordList.clear();
  jsonReadArray(settingsFlashJson, "routerssid", _ssidList);
  jsonReadArray(settingsFlashJson, "routerpass", _passwordList);
  int16_t numNetworks = WiFi.scanComplete();
  if (numNetworks <= 0)
  {
    SerialPrint("i", "WIFI", "no networks found");
    return;
  }

  // Ищем известные сети
  int connectNumNet = -1;
  // SerialPrint("i", "WIFI", "Count found: "+numNetworks);
  for (int n = 0; n < numNetworks; ++n)
  {
    String ssid = WiFi.SSID(n);
    jsonWriteStr_(ssidListHeapJson, String(n), ssid);
    for (size_t i = 0; i < _ssidList.size(); i++)
    {
      if (ssid == _ssidList[i])
      {
        Serial.printf("Found known network: %s\n", _ssidList[i]);
        if (connectNumNet < 0)
          connectNumNet = i;
      }
    }
    // if
  }
  sendStringToWs("ssidli", ssidListHeapJson, -1);  
  SerialPrint("i", "WIFI", "Scan Found: " + ssidListHeapJson);
  if (connectNumNet >= 0 && !isNetworkActive())
  {
    // ts.remove(WIFI_SCAN);
    connectToNextNetwork();
  }
  // checkConnection();
  // connectToSTA(_ssidList[connectNumNet], _passwordList[connectNumNet]);
  WiFi.scanDelete();
}

void WiFiUtilsItit()
{
#if !defined LIBRETINY
#if defined(esp32c6_4mb) || defined(esp32c6_8mb)
  WiFi.setAutoReconnect(false);
#else
  WiFi.setAutoConnect(false);
#endif
  WiFi.persistent(true); // Сохраняет текущую сеть при сканировании
  WiFi.setSleep(true);
#endif
  WiFi.mode(WIFI_STA);
  WiFi.onEvent(WiFiEvent);
  _ssidList.clear();
  _passwordList.clear();
  jsonReadArray(settingsFlashJson, "routerssid", _ssidList);
  jsonReadArray(settingsFlashJson, "routerpass", _passwordList);

  if (_ssidList.empty() || _passwordList.empty())
  {
    SerialPrint("E", "WIFI", "No networks configured");
    startAPMode();
    return;
  }
  currentNetwork = 0;
  connectionAttempts = 0;
  connectToNextNetwork();
}

void connectToNextNetwork()
{
  // все сети перебрали
  if (currentNetwork >= _ssidList.size())
  {
    SerialPrint("i", "WIFI", "All networks tried");
    ts.remove(WIFI_CONN);
    startAPMode();
    return;
  }

  wifiConnecting = true;
  // connectionAttempts++;

  const char *ssid = _ssidList[currentNetwork].c_str();
  const char *pass = _passwordList[currentNetwork].c_str();
  // Пробуем подключиться к сети
  SerialPrint("i", "WIFI", "Connecting to: " + String(ssid));
  WiFi.begin(ssid, pass);

#if defined(ESP32)
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
#elif defined(ESP8266)
  WiFi.setOutputPower(20.5);
#endif
  // проверяем статус подключения и перебираем сети если таймаут не вышел
  checkConnection();

  // wifiReconnectTicker.once_ms(WIFI_CHECK_INTERVAL, checkConnection);
}

void checkConnection()
{
  ts.add(
      WIFI_CONN, 1000,
      [&](void *)
      {
        connectionAttempts++;
        if (WiFi.status() == WL_CONNECTED)
        {
          connectionAttempts = 0;
          wifiConnecting = false;
          return;
        }

        if (connectionAttempts >= (_ssidList.size() > 1 ? TRIESONE : TRIES))
        {
          SerialPrint("i", "WIFI", "Max attempts reached");
          currentNetwork++;
          connectionAttempts = 0;
          wifiConnecting = false;
        }

        if (wifiConnecting)
        {
#ifdef ESP8266
          if (WiFi.status() == WL_CONNECT_FAILED || WiFi.status() == WL_WRONG_PASSWORD)
#else
          if (WiFi.status() == WL_CONNECT_FAILED)
#endif
          {
            SerialPrint("E", "WIFI", "Connection failed, wrong password?");
            jsonWriteInt(errorsHeapJson, "passer", 1);
            currentNetwork++;
            connectionAttempts = 0;
          }

          // wifiReconnectTicker.once_ms(WIFI_CHECK_INTERVAL, checkConnection);
        }

        if (!wifiConnecting)
        {
          connectToNextNetwork();
        }
      },
      nullptr, true);
}

//------------------------------------------
// Неблокирующее подключение к STA
//------------------------------------------
void connectToSTA(const char *ssid, const char *pass)
{
  if (isNetworkActive())
    return;
  SerialPrint("i", "WIFI", "Connecting to ... " + String(ssid));
  // SerialPrint("i", "WIFI", "pass connect: " + _passwordList[i]);
  WiFi.begin(ssid, pass);
#if defined(ESP32)
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
#elif defined(ESP8266)
  WiFi.setOutputPower(20.5);
#endif
}

void ScanAsync()
{
  // bool res = false;
  int n = WiFi.scanComplete();
  SerialPrint("i", "WIFI", "scan result: " + String(n, DEC));

  if (n == -1)
  { // Сканирование все еще выполняется
    SerialPrint("i", "WIFI", "scanning in progress");
  }
  else
  {
    SerialPrint("i", "WIFI", "start scanning");
    WiFi.scanNetworks(true, false);
  }
}
#else //WIFI_ASYNC

void routerConnect()
{
#if defined(esp32_wifirep)
//  Set custom dns server address for dhcp server
#define MY_DNS_IP_ADDR 0xC0A80401 // 192.168.4.1 // 0x08080808 // 8.8.8.8
  ip_addr_t dnsserver;

  String _ssidAP = jsonReadStr(settingsFlashJson, "apssid");
  String _passwordAP = jsonReadStr(settingsFlashJson, "appass");
  int _chanelAP = 0;
  jsonRead(settingsFlashJson, "wifirep_apchanel", _chanelAP);
  if (_chanelAP == 0)
    _chanelAP = 7;

  // WiFi.begin(ssid, password);
  WiFi.mode(WIFI_AP_STA);

  String s_apip = "";
  bool ap_ip = jsonRead(settingsFlashJson, "wifirep_apip", s_apip);
  if (ap_ip && s_apip != "")
  {
    WiFi.softAPConfig(stringToIp(s_apip), stringToIp(s_apip), stringToIp("255.255.255.0"));
    // bool softAPConfig(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dhcp_lease_start = (uint32_t) 0);
    dnsserver.u_addr.ip4.addr = stringToIp(s_apip);
  }
  else
    dnsserver.u_addr.ip4.addr = htonl(MY_DNS_IP_ADDR);

  dnsserver.type = IPADDR_TYPE_V4;
  dhcps_dns_setserver(&dnsserver);

  WiFi.softAP(_ssidAP.c_str(), _passwordAP.c_str(), _chanelAP, 0, 5);
  jsonWriteStr(settingsFlashJson, "ip", WiFi.softAPIP().toString());
  SerialPrint("i", "WIFI", "AP SSID: " + WiFi.softAPSSID());
  SerialPrint("i", "WIFI", "AP IP: " + WiFi.softAPIP().toString());
  SerialPrint("i", "WIFI", "AP pass: " + _passwordAP);

  String s_staip = "";
  bool static_ip = jsonRead(settingsFlashJson, "wifirep_staip", s_staip);
  String s_gateway = jsonReadStr(settingsFlashJson, "wifirep_gateway");
  String s_netmask = jsonReadStr(settingsFlashJson, "wifirep_netmask");
  String s_dns = jsonReadStr(settingsFlashJson, "wifirep_dns");

  if (static_ip == true && s_staip != "")
  {
    SerialPrint("i", "WIFI", "Use static IP");
    WiFi.config(stringToIp(s_staip), stringToIp(s_gateway), stringToIp(s_netmask), stringToIp(s_dns));
    // bool config(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dns1 = (uint32_t)0x00000000, IPAddress dns2 = (uint32_t)0x00000000);
    SerialPrint("i", "WIFI", "Static IP: " + s_staip);
    SerialPrint("i", "WIFI", "Gateway: " + s_gateway);
    SerialPrint("i", "WIFI", "Netmask: " + s_netmask);
    SerialPrint("i", "WIFI", "DNS: " + s_dns);
  }
#else
  WiFi.mode(WIFI_STA);
#endif

#if  !defined LIBRETINY  
#if defined(esp32c6_4mb) || defined(esp32c6_8mb)
  WiFi.setAutoReconnect(false);
#else
  WiFi.setAutoConnect(false);
#endif
  WiFi.persistent(false);
#endif
/*     String s_staip = "192.168.2.62";
    String s_gateway = "192.168.2.1";
    String s_netmask = "255.255.255.0";
    String s_dns = "192.168.2.1";
    SerialPrint("i", "WIFI", "Use static IP");
    WiFi.config(stringToIp(s_staip), stringToIp(s_gateway), stringToIp(s_netmask), stringToIp(s_dns));
    // bool config(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dns1 = (uint32_t)0x00000000, IPAddress dns2 = (uint32_t)0x00000000);
    SerialPrint("i", "WIFI", "Static IP: " + s_staip);
    SerialPrint("i", "WIFI", "Gateway: " + s_gateway);
    SerialPrint("i", "WIFI", "Netmask: " + s_netmask);
    SerialPrint("i", "WIFI", "DNS: " + s_dns); */
  //WiFi.mode(WIFI_STA);
  byte triesOne = TRIESONE;

  std::vector<String> _ssidList;
  std::vector<String> _passwordList;
  jsonReadArray(settingsFlashJson, "routerssid", _ssidList);
  jsonReadArray(settingsFlashJson, "routerpass", _passwordList);
  if (_ssidList.size() > 1)
    triesOne = TRIES;

  if (_passwordList.size() == 0 && _ssidList[0] == "" && _passwordList[0] == "")
  {
    #ifndef LIBRETINY
    WiFi.begin();
    #endif
  }
  else
  {
    WiFi.begin(_ssidList[0].c_str(), _passwordList[0].c_str());
#if defined (ESP32)
    WiFi.setTxPower(WIFI_POWER_19_5dBm);
#elif defined (ESP8266)
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
    if (WiFi.status() == WL_CONNECTED)
      break;
    WiFi.begin(_ssidList[i].c_str(), _passwordList[i].c_str());
    SerialPrint("i", "WIFI", "ssid connect: " + _ssidList[i]);
    SerialPrint("i", "WIFI", "pass connect: " + _passwordList[i]);
    while (--triesOne && WiFi.status() != WL_CONNECTED)
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
#if defined(ESP32)
      //SerialPrint("i", "Task", "Resetting WDT...");
       #if !defined(esp32c6_4mb) && !defined(esp32c6_8mb) //TODO esp32-c6 переписать esp_task_wdt_init
      esp_task_wdt_reset();
      #endif
#endif
      Serial.print(".");
      delay(1000);
    }
    Serial.println("");
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("");
    startAPMode();
  }
  else
  {
    Serial.println("");
#ifdef LIBRETINY
    SerialPrint("i", "WIFI", "http://" + ipToString(WiFi.localIP()));
    jsonWriteStr(settingsFlashJson, "ip", ipToString(WiFi.localIP()));
#else
    SerialPrint("i", "WIFI", "http://" + WiFi.localIP().toString());
    jsonWriteStr(settingsFlashJson, "ip", WiFi.localIP().toString());
#endif
    createItemFromNet("onWifi", "1", 1);

#if defined(esp32_wifirep)
    // Enable DNS (offer) for dhcp server
    dhcps_offer_t dhcps_dns_value = OFFER_DNS;
    dhcps_set_option_info(6, &dhcps_dns_value, sizeof(dhcps_dns_value));
    u32_t napt_netif_ip;
    if (ap_ip && s_apip != "")
      napt_netif_ip = stringToIp(s_apip);
    else
    {
      napt_netif_ip = 0xC0A80401; // Set to ip address of softAP netif (Default is 192.168.4.1)
      napt_netif_ip = htonl(napt_netif_ip);
    }
    // get_esp_interface_netif(ESP_IF_WIFI_AP)
    ip_napt_enable(napt_netif_ip, 1);
    // ip_napt_enable_no(ESP_IF_WIFI_AP, 1);

#endif

    mqttInit();
  }
  SerialPrint("i", F("WIFI"), F("Network Init"));
}
#endif
bool startAPMode()
{
#ifdef WIFI_ASYNC
  wifiConnecting = false;
  currentNetwork = 0;
  connectionAttempts = 0;
#endif
  SerialPrint("i", "WIFI", "AP Mode");

  WiFi.disconnect();
  WiFi.mode(WIFI_AP);

  String _ssidAP = jsonReadStr(settingsFlashJson, "apssid");
  String _passwordAP = jsonReadStr(settingsFlashJson, "appass");
  if (_passwordAP == "")
    WiFi.softAP(_ssidAP.c_str(), NULL, 6);
  else
    WiFi.softAP(_ssidAP.c_str(), _passwordAP.c_str(), 6);
  IPAddress myIP = WiFi.softAPIP();
#ifdef LIBRETINY
  SerialPrint("i", "WIFI", "AP IP: " + ipToString(myIP));
  jsonWriteStr(settingsFlashJson, "ip", ipToString(myIP));
#else
  SerialPrint("i", "WIFI", "AP IP: " + myIP.toString());
  jsonWriteStr(settingsFlashJson, "ip", myIP.toString());
#endif
  if (jsonReadInt(errorsHeapJson, "passer") != 1)
  {
    ts.add(
        WIFI_SCAN, 30 * 1000,
        [&](void *)
        {
#ifndef WIFI_ASYNC
          std::vector<String> jArray;
          jsonReadArray(settingsFlashJson, "routerssid", jArray);
          for (int8_t i = 0; i < jArray.size(); i++)
          {
            SerialPrint("i", "WIFI", "scanning for " + jArray[i]);
          }
          if (RouterFind(jArray))
          {
            ts.remove(WIFI_SCAN);
            WiFi.scanDelete();
            routerConnect();
          }
#else
          ScanAsync();
#endif
        },
        nullptr, true);
  }
  return true;
}

#ifndef WIFI_ASYNC
boolean RouterFind(std::vector<String> jArray)
{
  bool res = false;
  int n = WiFi.scanComplete();
  SerialPrint("i", "WIFI", "scan result: " + String(n, DEC));

  if (n == -2)
  { // Сканирование не было запущено, запускаем
    SerialPrint("i", "WIFI", "start scanning");
    WiFi.scanNetworks(true, false); // async, show_hidden
  }

  else if (n == -1)
  { // Сканирование все еще выполняется
    SerialPrint("i", "WIFI", "scanning in progress");
  }

  else if (n == 0)
  { // ни одна сеть не найдена
    SerialPrint("i", "WIFI", "no networks found");
    WiFi.scanNetworks(true, false);
  }
  else if (n > 0)
  {
    for (int8_t i = 0; i < n; i++)
    {
      for (int8_t k = 0; k < jArray.size(); k++)
      {
        if (WiFi.SSID(i) == jArray[k])
        {
          res = true;
        }
      }
      // SerialPrint("i", "WIFI", (res ? "*" : "") + String(i, DEC) + ") " + WiFi.SSID(i));
      jsonWriteStr_(ssidListHeapJson, String(i), WiFi.SSID(i));

      // String(WiFi.RSSI(i)
    }
  }
  SerialPrint("i", "WIFI", ssidListHeapJson);
  WiFi.scanDelete();
  return res;
}
#endif

boolean isNetworkActive()
{
  return WiFi.status() == WL_CONNECTED;
}

uint8_t getNumAPClients()
{
  return WiFi.softAPgetStationNum();
}

uint8_t RSSIquality()
{
  uint8_t res = 0;
  if (isNetworkActive())
  {
    int rssi = WiFi.RSSI();
    if (rssi >= -50)
    {
      res = 6; //"Excellent";
    }
    else if (rssi < -50 && rssi >= -60)
    {
      res = 5; //"Very good";
    }
    else if (rssi < -60 && rssi >= -70)
    {
      res = 4; //"Good";
    }
    else if (rssi < -70 && rssi >= -80)
    {
      res = 3; //"Low";
    }
    else if (rssi < -80 && rssi > -100)
    {
      res = 2; //"Very low";
    }
    else if (rssi <= -100)
    {
      res = 1; //"No signal";
    }
  }
  return res;
}

#ifdef LIBRETINY
String httpGetString(HTTPClient &http)
{
  String payload = "";
  int len = http.getSize();
  uint8_t buff[128] = {0};
  WiFiClient *stream = http.getStreamPtr();

  // read all data from server
  while (http.connected() && (len > 0 || len == -1))
  {
    // get available data size
    size_t size = stream->available();

    if (size)
    {
      // read up to 128 byte
      int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));

      // write it to Serial
      //   Serial.write(buff,c);

      // payload += String((char*)buff);
      char charBuff[c + 1];        // Create a character array with space for null terminator
      memcpy(charBuff, buff, c);   // Copy the data to the character array
      charBuff[c] = '\0';          // Null-terminate the character array
      payload += String(charBuff); // Append the character array to the payload

      if (len > 0)
      {
        len -= c;
      }
    }
    delay(1);
  }
  return payload;
}
#endif