#include "Global.h"
#include "classes/IoTItem.h"
#include <ArduinoJson.h>

long prevWeatherMillis = millis() - 60001;
//StaticJsonDocument<JSON_BUFFER_SIZE * 2> Weatherdoc;

extern IoTGpio IoTgpio;
class Weather : public IoTItem
{
private:
    String _location;
    String _param;
    // long interval;
    DynamicJsonDocument Weatherdoc;
public:
    Weather(String parameters) : Weatherdoc(1024), IoTItem(parameters)
    {
        _location = jsonReadStr(parameters, "location");
        _param = jsonReadStr(parameters, "param");
        long interval;
        jsonRead(parameters, F("int"), interval); // интервал проверки погоды в часах
        setInterval(interval * 60 * 60);
    }

    void getWeather()
    {
        String ret;

        if (isNetworkActive())
        {
            // char c;
            String payload;
            WiFiClient client;
            HTTPClient http;
            http.begin(client, "http://live-control.com/iotm/weather.php");
            http.addHeader("Content-Type", "application/x-www-form-urlencoded");
            String httpRequestData = "loc=" + _location;
            int httpResponseCode = http.POST(httpRequestData);
            if (httpResponseCode > 0)
            {
                ret = httpResponseCode;

                if (httpResponseCode == HTTP_CODE_OK)
                {
                    payload = http.getString();

                    deserializeJson(Weatherdoc, payload);
                    // ret += payload;
                }
            }
            else
            {
                ret = http.errorToString(httpResponseCode).c_str();
            }
            SerialPrint("<-", F("getWeather"), httpRequestData);
            SerialPrint("->", F("getWeather"), "server: " + ret);

            http.end();
        }
    }

    void doByInterval()
    {

        if (prevWeatherMillis + 60000 < millis())
        {
            getWeather();
            prevWeatherMillis = millis();
        }
        if (jsonReadStr(Weatherdoc["current_condition"][0], "temp_C", true) != "null")
        {
            if (_param == "temp_C")
            {
                value.valS = jsonReadStr(Weatherdoc["current_condition"][0], "temp_C", true);
            }
            if (_param == "avgtempC")
            {
                value.valS = jsonReadStr(Weatherdoc["weather"][0], "avgtempC", true);
            }
            if (_param == "humidity")
            {
                value.valS = jsonReadStr(Weatherdoc["current_condition"][0], "humidity", true);
            }
            if (_param == "weatherCode")
            {
                value.valS = jsonReadStr(Weatherdoc["current_condition"][0], "weatherCode", true);
            }
            if (_param == "sunrise")
            {

                value.valS = jsonReadStr(Weatherdoc["weather"][0]["astronomy"][0], "sunrise", true);
            }
            if (_param == "sunset")
            {
                value.valS = jsonReadStr(Weatherdoc["weather"][0]["astronomy"][0], "sunset", true);
            }

            if (_param == "rangetempC")
            {
                value.valS = jsonReadStr(Weatherdoc["weather"][0], "mintempC", true) + "..." + jsonReadStr(Weatherdoc["weather"][0], "maxtempC", true);
            }

            // погода на завтра
            if (_param == "temp_C_tomorrow")
            {
                value.valS = jsonReadStr(Weatherdoc["weather"][1], "avgtempC", true);
            }
            if (_param == "rangetempC_tomorrow")
            {
                value.valS = jsonReadStr(Weatherdoc["weather"][1], "mintempC", true) + "..." + jsonReadStr(Weatherdoc["weather"][1], "maxtempC", true);
            }

            regEvent(value.valS, "Weather");
        }
    }

    // void loop()
    // {
    //     if (enableDoByInt)
    //     {
    //         currentMillis = millis();
    //         difference = currentMillis - prevMillis;
    //         if (difference >= interval)
    //         {
    //             prevMillis = millis();
    //             this->doByInterval();
    //         }
    //     }
    // }

    IoTValue execute(String command, std::vector<IoTValue> &param)
    {
        if (command == "get")
        {
            getWeather();
            doByInterval();
        }

        return {};
    }

    ~Weather(){};
};

void *getAPI_Weather(String subtype, String param)
{
    if (subtype == F("Weather"))
    {
        return new Weather(param);
    }
    else
    {
        return nullptr;
    }
}
