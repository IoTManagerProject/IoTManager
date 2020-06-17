#include "TimeUtils.h"

#include "main.h"
#include "set.h"

void Time_Init() {
    server.on("/time", HTTP_GET, [](AsyncWebServerRequest* request) {
        if (request->hasArg("timezone")) {
            jsonWriteStr(configSetup, "timezone", request->getParam("timezone")->value());
        }
        if (request->hasArg("ntp")) {
            jsonWriteStr(configSetup, "ntp", request->getParam("ntp")->value());
        }
        saveConfig();
        reconfigTime();
        request->send(200, "text/text", "OK");
    });

    ts.add(
        TIME_SYNC, 30000, [&](void*) {
            time_check();
        },
        nullptr, true);
}

void time_check() {
    if (GetTimeUnix() == "failed") {
        Serial.println("[i] Time is not synchronized, start synchronization");
        reconfigTime();
    }
}

void reconfigTime() {
    if (WiFi.status() == WL_CONNECTED) {
        String ntp = jsonReadStr(configSetup, "ntp");
        configTime(0, 0, ntp.c_str());
        int i = 0;
        Serial.println("[i] Awaiting for time ");
#ifdef ESP32
        struct tm timeinfo;
        while (!getLocalTime(&timeinfo) && i <= 4) {
            Serial.print(".");
            i++;
            delay(1000);
        }
#endif
#ifdef ESP8266
        //while (!time(nullptr) && i < 4) {
        //  Serial.print(".");
        //  i++;
        delay(2000);
        //}
#endif
        if (GetTimeUnix() != "failed") {
            Serial.print("[V] Time synchronized = ");
            Serial.print(GetDataDigital());
            Serial.print(" ");
            Serial.println(GetTime());
        } else {
            Serial.println("[E] Time server or internet connection error, will try again in 30 sec");
        }
    } else {
        Serial.println("[E] Get time impossible, no wifi connection");
    }
}

//Получаем время в формате linux gmt
String GetTimeUnix() {
    time_t now = time(nullptr);
    if (now < 30000) {
        return "failed";
    } else {
        return String(now);
    }
}

// Получение текущего времени
String GetTime() {
    time_t now = time(nullptr);  // получаем время с помощью библиотеки time.h
    int zone = 3600 * jsonReadStr(configSetup, "timezone").toInt();
    now = now + zone;
    String Time = "";                     // Строка для результатов времени
    Time += ctime(&now);                  // Преобразуем время в строку формата Thu Jan 19 00:55:35 2017
    int i = Time.indexOf(":");            //Ишем позицию первого символа :
    Time = Time.substring(i - 2, i + 6);  // Выделяем из строки 2 символа перед символом : и 6 символов после
    return Time;                          // Возврашаем полученное время
}

String GetTimeWOsec() {
    time_t now = time(nullptr);  // получаем время с помощью библиотеки time.h
    int zone = 3600 * jsonReadStr(configSetup, "timezone").toInt();
    now = now + zone;
    String Time = "";                     // Строка для результатов времени
    Time += ctime(&now);                  // Преобразуем время в строку формата Thu Jan 19 00:55:35 2017
    int i = Time.indexOf(":");            //Ишем позицию первого символа :
    Time = Time.substring(i - 2, i + 3);  // Выделяем из строки 2 символа перед символом : и 6 символов после
    return Time;                          // Возврашаем полученное время
}

// Получение даты
String GetDate() {
    time_t now = time(nullptr);  // получаем время с помощью библиотеки time.h
    int zone = 3600 * jsonReadStr(configSetup, "timezone").toInt();
    now = now + zone;
    String Data = "";     // Строка для результатов времени
    Data += ctime(&now);  // Преобразуем время в строку формата Thu Jan 19 00:55:35 2017
    Data.replace("\n", "");
    uint8_t i = Data.lastIndexOf(" ");           //Ишем позицию последнего символа пробел
    String Time = Data.substring(i - 8, i + 1);  // Выделяем время и пробел
    Data.replace(Time, "");                      // Удаляем из строки 8 символов времени и пробел
    return Data;                                 // Возврашаем полученную дату
}

String GetDataDigital() {
    String date = GetDate();
    date = deleteBeforeDelimiter(date, " ");
    date.replace("Jan", "01");
    date.replace("Feb", "02");
    date.replace("Mar", "03");
    date.replace("Apr", "04");
    date.replace("May", "05");
    date.replace("Jun", "06");
    date.replace("Jul", "07");
    date.replace("Aug", "08");
    date.replace("Sep", "09");
    date.replace("Oct", "10");
    date.replace("Nov", "11");
    date.replace("Dec", "12");

    String month = date.substring(0, 2);
    String day = date.substring(3, 5);
    String year = date.substring(8, 10);

    String out = day;
    out += ".";
    out += month;
    out += ".";
    out += year;

    return out;
}

int timeToMin(String Time) {
    //"00:00:00"  время в секунды
    long min = selectToMarker(Time, ":").toInt() * 60;  //общее количество секунд в полных часах
    Time = deleteBeforeDelimiter(Time, ":");            // Теперь здесь минуты секунды
    min += selectToMarker(Time, ":").toInt();           // Добавим секунды из полных минут
    return min;
}
