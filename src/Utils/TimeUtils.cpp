#include "Utils\TimeUtils.h"

#include "Global.h"
#include "Utils\StringUtils.h"

#define ONE_MINUTE_s 60
#define ONE_HOUR_s 60 * ONE_MINUTE_s

int getBiasInSeconds() {
    return 3600 * jsonReadStr(configSetupJson, "timezone").toInt();
}

int getBiasInMinutes() {
    return getBiasInSeconds() / 60;
}

const timezone getTimeZone() {
    return timezone{getBiasInMinutes(), 0};
}

time_t getSystemTime() {
    timeval tv{0, 0};
    timezone tz = getTimeZone();
    time_t epoch = 0;
    if (gettimeofday(&tv, &tz) != -1)
        epoch = tv.tv_sec;
    return epoch;
}

bool hasTimeSynced() {
    unsigned long now = time(nullptr);
    return now > millis();
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
    time_t now = time(nullptr);
    int zone = 3600 * jsonReadStr(configSetupJson, "timezone").toInt();
    now = now + zone;
    String Time = "";                     // Строка для результатов времени
    Time += ctime(&now);                  // Преобразуем время в строку формата Thu Jan 19 00:55:35 2017
    int i = Time.indexOf(":");            //Ишем позицию первого символа :
    Time = Time.substring(i - 2, i + 6);  // Выделяем из строки 2 символа перед символом : и 6 символов после
    return Time;                          // Возврашаем полученное время
}

String getTimeWOsec() {
    time_t now = time(nullptr);
    int zone = 3600 * jsonReadStr(configSetupJson, "timezone").toInt();
    now = now + zone;
    String Time = "";                     // Строка для результатов времени
    Time += ctime(&now);                  // Преобразуем время в строку формата Thu Jan 19 00:55:35 2017
    int i = Time.indexOf(":");            //Ишем позицию первого символа :
    Time = Time.substring(i - 2, i + 3);  // Выделяем из строки 2 символа перед символом : и 6 символов после
    return Time;                          // Возврашаем полученное время
}

String getDate() {
    time_t now = time(nullptr);
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

String getDateDigitalFormated() {
    String date = getDate();

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

static const char* TIME_FORMAT PROGMEM = "%02d:%02d:%02d";
static const char* TIME_FORMAT_WITH_DAYS PROGMEM = "%dd %02d:%02d";

const String prettyMillis(unsigned long time_ms) {
    unsigned long tmp = time_ms / 1000;
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

    char buf[32];

    if (days) {
        sprintf_P(buf, TIME_FORMAT_WITH_DAYS, days, hours, minutes, seconds);
    } else {
        sprintf_P(buf, TIME_FORMAT, hours, minutes, seconds);
    }
    return String(buf);
}

int timeZoneInSeconds(const byte timeZone) {
    int res = 0;
    switch (constrain(timeZone, 1, 38)) {
        case 1:
            res = -12 * ONE_HOUR_s;
            break;
        case 2:
            res = -11 * ONE_HOUR_s;
            break;
        case 3:
            res = -10 * ONE_HOUR_s;
            break;
        case 4:
            res = -9 * ONE_HOUR_s - 30 * ONE_MINUTE_s;
            break;
        case 5:
            res = -9 * ONE_HOUR_s;
            break;
        case 6:
            res = -8 * ONE_HOUR_s;
            break;
        case 7:
            res = -7 * ONE_HOUR_s;
            break;
        case 8:
            res = -6 * ONE_HOUR_s;
            break;
        case 9:
            res = -5 * ONE_HOUR_s;
            break;
        case 10:
            res = -4 * ONE_HOUR_s;
            break;
        case 11:
            res = -3 * ONE_HOUR_s - 30 * ONE_MINUTE_s;
            break;
        case 12:
            res = -3 * ONE_HOUR_s;
            break;
        case 13:
            res = -2 * ONE_HOUR_s;
            break;
        case 14:
            res = -1 * ONE_HOUR_s;
            break;
        case 15:
            res = 0;
            break;
        case 16:
            res = 1 * ONE_HOUR_s;
            break;
        case 17:
            res = 2 * ONE_HOUR_s;
            break;
        case 18:
            res = 3 * ONE_HOUR_s;
            break;
        case 19:
            res = 3 * ONE_HOUR_s + 30 * ONE_MINUTE_s;
            break;
        case 20:
            res = 4 * ONE_HOUR_s;
            break;
        case 21:
            res = 4 * ONE_HOUR_s + 30 * ONE_MINUTE_s;
            break;
        case 22:
            res = 5 * ONE_HOUR_s;
            break;
        case 23:
            res = 5 * ONE_HOUR_s + 30 * ONE_MINUTE_s;
            break;
        case 24:
            res = 5 * ONE_HOUR_s + 45 * ONE_MINUTE_s;
            break;
        case 25:
            res = 6 * ONE_HOUR_s;
            break;
        case 26:
            res = 6 * ONE_HOUR_s + 30 * ONE_MINUTE_s;
            break;
        case 27:
            res = 7 * ONE_HOUR_s;
            break;
        case 28:
            res = 8 * ONE_HOUR_s;
            break;
        case 29:
            res = 8 * ONE_HOUR_s + 45 * ONE_MINUTE_s;
            break;
        case 30:
            res = 9 * ONE_HOUR_s;
            break;
        case 31:
            res = 9 * ONE_HOUR_s + 30 * ONE_MINUTE_s;
            break;
        case 32:
            res = 10 * ONE_HOUR_s;
            break;
        case 33:
            res = 10 * ONE_HOUR_s + 30 * ONE_MINUTE_s;
            break;
        case 34:
            res = 11 * ONE_HOUR_s;
            break;
        case 35:
            res = 12 * ONE_HOUR_s;
            break;
        case 36:
            res = 12 * ONE_HOUR_s + 45 * ONE_MINUTE_s;
            break;
        case 37:
            res = 13 * ONE_HOUR_s;
            break;
        case 38:
            res = 14 * ONE_HOUR_s;
            break;
    }
    return res;
}

unsigned long millis_since(unsigned long sinse) {
    return millis_passed(sinse, millis());
}

unsigned long millis_passed(unsigned long start, unsigned long finish) {
    unsigned long result = 0;
    if (start <= finish) {
        unsigned long passed = finish - start;
        if (passed <= __LONG_MAX__) {
            result = static_cast<long>(passed);
        } else {
            result = static_cast<long>((__LONG_MAX__ - finish) + start + 1u);
        }
    } else {
        unsigned long passed = start - finish;
        if (passed <= __LONG_MAX__) {
            result = static_cast<long>(passed);
            result = -1 * result;
        } else {
            result = static_cast<long>((__LONG_MAX__ - start) + finish + 1u);
            result = -1 * result;
        }
    }
    return result;
}
