
#include "Global.h"
#include "classes/IoTItem.h"

String URL = "https://script.google.com/macros/s/";


class GoogleSheet : public IoTItem
{
private:
  String id;
  String logid;
  String scid = "";
  String shname = "";
  bool init = false;
  int interval = 1;
 // long interval;
public:
  GoogleSheet(String parameters) : IoTItem(parameters)
  {
    jsonRead(parameters, F("id"), id);
    jsonRead(parameters, F("logid"), logid);
    jsonRead(parameters, F("scid"), scid);
    jsonRead(parameters, F("shname"), shname);
    jsonRead(parameters, F("int"), interval);
    interval = interval * 1000 * 60; // так как у нас в минутах
  }

  void doByInterval()
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      String value = getItemValue(logid);
      String urlFinal = URL + scid + "/exec?"+"sheet=" + shname + "&id=" + logid + "&value=" + value;
      if (!init){ init=true; urlFinal = urlFinal + "&init=1";}

      HTTPClient http;
#if defined ESP8266
      WiFiClientSecure client;
      client.setInsecure();
      if (!http.begin(client, urlFinal))
      {
#elif defined ESP32      
      WiFiClient client;
      if (!http.begin(urlFinal))
      {
#endif
        SerialPrint("I", F("GoogleSheet"), "connection failed  ");
      }
      http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS); // HTTPC_STRICT_FOLLOW_REDIRECTS HTTPC_FORCE_FOLLOW_REDIRECTS
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      int httpCode = http.GET();
//      String payload = http.getString();
      SerialPrint("<-", F("GoogleSheet"), "URL: " + urlFinal);
      SerialPrint("->", F("GoogleSheet"), "server: " + (String)httpCode); /*"URL: " + urlFinal + */
/*      if (httpCode > 0)
      {
        SerialPrint("->", F("GoogleSheet"), "msg from server: " + (String)payload.c_str());
      }
*/
      http.end();
    }
  }

  void loop()
  {
    if (enableDoByInt)
    {
      currentMillis = millis();
      difference = currentMillis - prevMillis;
      if (difference >= interval)
      {
        prevMillis = millis();
        this->doByInterval();
      }
    }
  }
  ~GoogleSheet(){};
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
