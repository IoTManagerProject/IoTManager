#include "Utils\TimeUtils.h"

#include "Global.h"
#include "Utils\StringUtils.h"

static const char* TIME_FORMAT PROGMEM = "%2d:%2d:%2d";
static const char* TIME_FORMAT_WITH_DAYS PROGMEM = "%dd %2d:%2d";

void Time_Init() {
    ts.add(
        TIME_SYNC, 30000, [&](void*) {
            time_check();
        },
        nullptr, true);
}

const String prettyMillis(unsigned long time_ms) {
    unsigned long tmp = time_ms;
    unsigned long seconds;
    unsigned long minutes;
    unsigned long hours;
    unsigned long days;
    seconds = tmp % 60;
    tmp = tmp / 60;

    minutes = tmp % 60;
    tmp = tmp / 60;
    hours = tmp % 24;
    days = tmp / 24;

    char buf[16];

    if (days) {
        sprintf_P(buf, TIME_FORMAT, hours, minutes, seconds);
    } else {
        sprintf_P(buf, TIME_FORMAT_WITH_DAYS, days, hours, minutes);
    }
    return String(buf);
}

void time_check() {
    if (getTimeUnix() == "failed") {
        Serial.println("[i] Time is not synchronized, start synchronization");
        reconfigTime();
    }
}

void reconfigTime() {
    if (WiFi.status() == WL_CONNECTED) {
        String ntp = jsonReadStr(configSetupJson, "ntp");
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
        if (getTimeUnix() != "failed") {
            Serial.print("[V] Time synchronized = ");
            Serial.print(GetDataDigital());
            Serial.print(" ");
            Serial.println(getTime());
        } else {
            Serial.println("[E] Time server or internet connection error, will try again in 30 sec");
        }
    } else {
        Serial.println("[E] Get time impossible, no wifi connection");
    }
}

String getTimeUnix() {
    time_t now = time(nullptr);
    if (now < 30000) {
        return "failed";
    } else {
        return String(now);
    }
}

boolean getUnixTimeStr(String& res) {
    time_t now = time(nullptr);
    res = String(now);
    return now < 30000;
}

String getTime() {
    time_t now = time(nullptr);  // получаем время с помощью библиотеки time.h
    int zone = 3600 * jsonReadStr(configSetupJson, "timezone").toInt();
    now = now + zone;
    String Time = "";                     // Строка для результатов времени
    Time += ctime(&now);                  // Преобразуем время в строку формата Thu Jan 19 00:55:35 2017
    int i = Time.indexOf(":");            //Ишем позицию первого символа :
    Time = Time.substring(i - 2, i + 6);  // Выделяем из строки 2 символа перед символом : и 6 символов после
    return Time;                          // Возврашаем полученное время
}

String GetTimeWOsec() {
    time_t now = time(nullptr);  // получаем время с помощью библиотеки time.h
    int zone = 3600 * jsonReadStr(configSetupJson, "timezone").toInt();
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
    int zone = 3600 * jsonReadStr(configSetupJson, "timezone").toInt();
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