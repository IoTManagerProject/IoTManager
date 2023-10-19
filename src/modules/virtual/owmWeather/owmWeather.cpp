#include "Global.h"
#include "classes/IoTItem.h"
#include <ArduinoJson.h>
#include "NTP.h"
// long prevWeatherMillis = millis() - 60001;
//  TODO Зачем так много???
// StaticJsonDocument<JSON_BUFFER_SIZE * 2> Weatherdoc1;

extern IoTGpio IoTgpio;
class owmWeather : public IoTItem
{
private:
    // описание параметров передаваемых из настроек датчика из веба

    String _location;
    String _param;
    //  long interval;
    String _API_key;
    String _city = "";
    String _lat = "";
    String _lon = "";
    String _lang = "";
    bool _debug = false;
    DynamicJsonDocument Weatherdoc1;
    unsigned long _sunsetTime = 0;
    unsigned long _sunriseTime = 0;
    uint32_t _tzone = 0;

public:
    owmWeather(String parameters) : Weatherdoc1(1024), IoTItem(parameters)
    {
        _API_key = jsonReadStr(parameters, "API_key");
        //    _ID_sity = jsonReadStr(parameters, "ID_sity");
        if (!jsonRead(parameters, "city", _city))
            _city = "";
        jsonRead(parameters, "lon", _lon);
        jsonRead(parameters, "lat", _lat);
        jsonRead(parameters, "lang", _lang);
        jsonRead(parameters, "param", _param);
        jsonRead(parameters, "debug", _debug);
        long interval;
        jsonRead(parameters, F("int"), interval); // в минутах
        setInterval(interval * 60);
    }

    void getWeather()
    {
        String ret;

        if (isNetworkActive())
        {
            String urlReq;
            if (_city != "")
            {
                // Для нового API
                //   request = "http://api.openweathermap.org/geo/1.0/direct?q=" _сity + "&limit=1&appid=" + _API_key;
                // Устарело, но пока работает
                urlReq = "http://api.openweathermap.org/data/2.5/weather?q=" + _city + "&units=metric&lang=" + _lang + "&appid=" + _API_key;
            }
            else
            {
                // Получаем название города по координатам. Новый API
                // request = "http://api.openweathermap.org/geo/1.0/reverse?lat=" + _lat + "&lon=" + _lon + "&limit=1&appid=" + _API_key;
                //[0].local_names:{ru:Москва,...}
                urlReq = "http://api.openweathermap.org/data/2.5/weather?lat=" + _lat + "&lon=" + _lon + "&units=metric&lang=" + _lang + "&appid=" + _API_key;
            }

            WiFiClient client;
            HTTPClient http;
            String payload;
            bool find = false;
            http.setTimeout(500);
            http.begin(client, urlReq); // urlCurrent
            // http.begin(client, "http://api.openweathermap.org/data/2.5/weather?id=" + _ID_sity + "&appid=" + _API_key + "&units=metric");
            http.addHeader("Content-Type", "application/x-www-form-urlencoded");
            int httpCode = http.GET();

            if (httpCode > 0)
            {
                ret = httpCode;

                if (httpCode == HTTP_CODE_OK)
                {
                    payload = http.getString();

                    deserializeJson(Weatherdoc1, payload);
                    // ret += payload;
                    if (_debug)
                        SerialPrint("i", "Weatherdoc1", "memoryUsage: " + String(Weatherdoc1.memoryUsage()));
                }
            }
            else
            {
                ret = http.errorToString(httpCode).c_str();
            }
            if (_debug)
            {
                SerialPrint("<-", F("getOwmWeather"), urlReq);
                SerialPrint("->", F("getOwmWeather"), "server: " + ret);
            }
            http.end();
        }
    }

    void doByInterval()
    {

        getWeather();
        if (jsonReadStr(Weatherdoc1["main"], "temp", true) != "null")
        {
            _tzone = Weatherdoc1["timezone"].as<int>();
            _sunriseTime = std::atoll(jsonReadStr(Weatherdoc1["sys"], "sunrise", true).c_str());
            _sunriseTime = _sunriseTime + _tzone;
            _sunsetTime = std::atoll(jsonReadStr(Weatherdoc1["sys"], "sunset", true).c_str());
            _sunsetTime = _sunsetTime + _tzone;

            publishNew("main", "temp");
            publishNew("main", "pressure");
            publishNew("main", "humidity");
            publishNew("wind", "speed");
            publishNew("wind", "deg");
            publishNew("clouds", "all");
            publishNew("weather", "main");
            publishNew("weather", "description");
            publishNew("weather", "icon");
            publishNew("sys", "sunrise");
            publishNew("sys", "sunset");
            publishNew("", "name");

            if (_param == "temp")
            {
                value.valS = jsonReadStr(Weatherdoc1["main"], "temp", true);
                regEvent(value.valS, "owmWeather");
            }
            else if (_param == "pressure")
            {
                // value.valS = jsonReadStr(Weatherdoc1["main"], "pressure", true);
                int tval;
                jsonRead(Weatherdoc1["main"], "pressure", tval, true);
                regEvent(tval / 1.333, "owmWeather");
            }
            else if (_param == "humidity")
            {
                value.valS = jsonReadStr(Weatherdoc1["main"], "humidity", true);
                regEvent(value.valS, "owmWeather");
            }
            else if (_param == "speed")
            {
                value.valS = jsonReadStr(Weatherdoc1["wind"], "speed", true);
                regEvent(value.valS, "owmWeather");
            }
            else if (_param == "deg")
            {
                value.valS = jsonReadStr(Weatherdoc1["wind"], "deg", true);
                regEvent(value.valS, "owmWeather");
            }
            else if (_param == "all")
            {
                value.valS = jsonReadStr(Weatherdoc1["clouds"], "all", true);
                regEvent(value.valS, "owmWeather");
            }
            else if (_param == "main")
            {
                value.valS = jsonReadStr(Weatherdoc1["weather"][0], "main", true);
                regEvent(value.valS, "owmWeather");
            }
            else if (_param == "description")
            {
                value.valS = jsonReadStr(Weatherdoc1["weather"][0], "description", true);
                regEvent(value.valS, "owmWeather");
            }
            else if (_param == "icon")
            {
                value.valS = jsonReadStr(Weatherdoc1["weather"][0], "icon", true);
                regEvent(value.valS, "owmWeather");
            }
            else if (_param == "sunrise")
            {
                value.valS = getTimeDotFormatedFromUnix(_sunriseTime);
                regEvent(value.valS, "owmWeather");
            }
            else if (_param == "sunset")
            {
                value.valS = getTimeDotFormatedFromUnix(_sunsetTime);
                regEvent(value.valS, "owmWeather");
            }
            else if (_param == "name")
            {
                value.valS = Weatherdoc1["name"].as<String>();
                regEvent(value.valS, "owmWeather");
            }
            // value.isDecimal = false;

            //     regEvent(value.valS, "owmWeather");
        }
    }

    IoTValue execute(String command, std::vector<IoTValue> &param)
    {
        IoTValue value = {};
        if (command == "get")
        {
            // getWeather();
            doByInterval();
        }
        else if (command == "sunset")
        {
            if (param.size())
            {
                if (param[0].isDecimal)
                    if ((unsigned long)(getSystemTime() + _tzone) >= (unsigned long)(_sunsetTime + (param[0].valD * 60)))
                        value.valD = 0;
                    else
                    {
                        if (_debug)
                        {
                            SerialPrint("i", ("owmWeather"), "set sunset astroTimer: " + String(_sunsetTime + param[0].valD * 60) + " date: " + getTimeDotFormatedFromUnix(_sunsetTime + param[0].valD * 60));
                        }
                        value.valD = (float)(((_sunsetTime + (int)(param[0].valD * 60)) - getSystemTime() + _tzone) / 60);
                    }
            }
            if (_debug)
            {
                SerialPrint("i", ("owmWeather"), "time: " + String(getSystemTime() + _tzone) + " sunset: " + String(_sunsetTime) + " Dt: " + String(param[0].valD) + " diff: " + String(value.valD));
            }
        }
        else if (command == "sunrise")
        {
            if (param.size())
            {
                if ((unsigned long)(getSystemTime() + _tzone) >= (unsigned long)(_sunriseTime + (param[0].valD * 60)))
                    value.valD = 0;
                else
                {
                    if (_debug)
                    {
                        SerialPrint("i", ("owmWeather"), "set sunrise astroTimer: " + String(_sunriseTime + param[0].valD * 60) + " date: " + getTimeDotFormatedFromUnix(_sunriseTime + param[0].valD * 60));
                    }
                    value.valD = (float)(((_sunriseTime + (int)(param[0].valD * 60)) - getSystemTime() + _tzone) / 60);
                }
            }
            if (_debug)
            {
                SerialPrint("i", ("owmWeather"), "time: " + String(getSystemTime() + _tzone) + " sunrise: " + String(_sunriseTime) + " Dt: " + String(param[0].valD) + " diff: " + String(value.valD));
            }
        }

        return value;
    }

    // проверяем если пришедшее значение отличается от предыдущего регистрируем событие
    void publishNew(String root, String param)
    {
        IoTItem *tmp = findIoTItem("wea_" + param);
        if (!tmp)
            return;

        if (root == "weather")
        { // ☔⛅❄🌤🌥🌦🌧🌨🌩🌫🌞
            String icn = Weatherdoc1[root][0]["icon"].as<String>();
            if (icn == "01d" || icn == "01n")
                icn = "🌞";
            else if (icn == "02d" || icn == "02n")
                icn = "🌤";
            else if (icn == "03d" || icn == "03n")
                icn = "🌥";
            else if (icn == "04d" || icn == "04n")
                icn = "🌫";
            else if (icn == "09d" || icn == "09n")
                icn = "🌧";
            else if (icn == "10d" || icn == "10n")
                icn = "🌦";
            else if (icn == "11d" || icn == "11n")
                icn = "🌩";
            else if (icn == "13d" || icn == "13n")
                icn = "❄";
            else
                icn = "";
            if (Weatherdoc1[root][0][param].as<String>() != tmp->value.valS)
            {
                if (param == "description")
                    tmp->setValue(Weatherdoc1[root][0][param].as<String>() + icn, true);
                else
                    tmp->setValue(Weatherdoc1[root][0][param].as<String>(), true);
            }
        }
        else if (root == "")
        {
            if (Weatherdoc1[param].as<String>() != tmp->value.valS)
            {
                tmp->setValue(Weatherdoc1[param].as<String>(), true);
            }
        }
        else if (root == "sys")
        {
            if (Weatherdoc1[root][param].as<String>() != tmp->value.valS)
            {
                if (param == "sunrise")
                {
                    tmp->setValue(getTimeDotFormatedFromUnix(_sunriseTime), true);
                }
                else if (param == "sunset")
                {
                    tmp->setValue(getTimeDotFormatedFromUnix(_sunsetTime), true);
                }
                else
                {
                    tmp->setValue(Weatherdoc1[root][param].as<String>(), true);
                }
            }
        }
        else
        {
            if (Weatherdoc1[root][param].as<String>() != tmp->value.valS)
            {
                if (param == "pressure")
                {
                    int tval = Weatherdoc1[root][param].as<int>();
                    tmp->setValue(String(tval / 1.333), true);
                }
                else
                {
                    tmp->setValue(Weatherdoc1[root][param].as<String>(), true);
                }
            }
        }
    }

    ~owmWeather(){};
};

void *getAPI_owmWeather(String subtype, String param)
{
    if (subtype == F("owmWeather"))
    {
        return new owmWeather(param);
    }
    else
    {
        return nullptr;
    }
}
