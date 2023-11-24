
#include "Global.h"
#include "classes/IoTItem.h"

class GoogleSheet : public IoTItem
{
private:
  String id;
  String logid;
  String scid = "";
  String shname = "";
  //  bool init = false;
  // int interval = 1;
  // long interval;
  String URL = ("http://iotmanager.org/projects/google.php?/macros/s/"); // F("https://script.google.com/macros/s/");
  String urlFinal;
  HTTPClient http;
  WiFiClient client;

public:
  GoogleSheet(String parameters) : IoTItem(parameters)
  {
    jsonRead(parameters, F("id"), id);
    jsonRead(parameters, F("logid"), logid);
    jsonRead(parameters, F("scid"), scid);
    jsonRead(parameters, F("shname"), shname);
    // jsonRead(parameters, F("int"), interval);
    long interval;
    jsonRead(parameters, F("int"), interval); // в минутах
    setInterval(interval * 60);
    // interval = interval * 1000 * 60; // так как у нас в минутах
    urlFinal = URL + scid + F("/exec?") + F("sheet=") + shname;
  }

  void doByInterval()
  {
    if (isNetworkActive())
    {
      String value = getItemValue(logid);
      if (value != "")
        send2Google(logid, value, true);
    }
  }

  // void loop()
  // {
  //   if (enableDoByInt)
  //   {
  //     currentMillis = millis();
  //     difference = currentMillis - prevMillis;
  //     if (difference >= interval)
  //     {
  //       prevMillis = millis();
  //       this->doByInterval();
  //     }
  //   }
  // }

  IoTValue execute(String command, std::vector<IoTValue> &param)
  {
    if (isNetworkActive())
    {
      if (command == F("logGoogle"))
      { // Логирование определенного элемента по его идентификатору в GoogleSheet
        /*        if (param.size() == 1)
                {
                  String id = param[0].valS;
                  String value = getItemValue(id);
                  send2Google(id, value, true);
                  return {};
                }
            */
        if (param.size() >= 1)
        {
          int sizeOfParam = param.size();
          for (unsigned int i = 0; i < sizeOfParam; i++)
          {
            IoTItem *itm = findIoTItemOnValue(param[i].valS);
            if (itm != nullptr)
            {
              if (i == sizeOfParam - 1)
                send2Google(itm->getID(), param[i].valS, true);
              else
                send2Google(itm->getID(), param[i].valS, false);
            }
          }
        }
      }
      else if (command == F("logGoogleAny"))
      { // Запись произвольных данных в GoogleSheet
        if (param.size() == 2)
        {
          send2Google(param[0].valS, param[1].valS, true);
          return {};
        }
      }
    }
    return {};
  }

  void send2Google(String logid, String value, bool send)
  {
    urlFinal = urlFinal + ("&id=") + logid + ("&value=") + value;

    if (!send)
      return; // Не отправляем просто накапливаем данные

      /*    if (!init)
          {
            init = true;
            urlFinal = urlFinal + "&init=1";
          }
      */
//    HTTPClient http;
#if defined ESP8266
    //    WiFiClient client;
    // client.setInsecure();
    if (!http.begin(client, urlFinal))
#elif defined ESP32
    //    WiFiClient client;
    if (!http.begin(urlFinal))
#endif
    {
      SerialPrint("I", F("GoogleSheet"), F("connection failed"));
    }
    //    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS); // HTTPC_STRICT_FOLLOW_REDIRECTS HTTPC_FORCE_FOLLOW_REDIRECTS
    http.addHeader(F("Content-Type"), F("application/x-www-form-urlencoded"));
    int httpCode = http.GET();

    String payload = http.getString();
    SerialPrint("<-", F("GoogleSheet"), "URL: " + urlFinal);
    SerialPrint("->", F("GoogleSheet"), "server: " + (String)httpCode); /*"URL: " + urlFinal + */

    /*      if (httpCode > 0)
          {
            SerialPrint("->", F("GoogleSheet"), "msg from server: " + (String)payload.c_str());
          }
    */
    //    http.end();
    // Обнуляем данные в запросе, так как все отправили
    urlFinal = URL + scid + F("/exec?") + F("sheet=") + shname;
  };

  // поиск элемента модуля в существующей конфигурации по его значению
  // По хорошему ЭТО наверное надо в ЯДРО пернести
  IoTItem *findIoTItemOnValue(const String &value)
  {
    if (value == "")
      return nullptr;
    for (std::list<IoTItem *>::iterator it = IoTItems.begin(); it != IoTItems.end(); ++it)
    {
      if ((*it)->getValue() == value)
        return *it;
    }

    return nullptr;
  };

  ~GoogleSheet() { http.end(); };
};

void *getAPI_GoogleSheet(String subtype, String param)
{

  if (subtype == F("GoogleSheet"))
  {
    return new GoogleSheet(param);
  }
  else
  {
    return nullptr;
  }
}
