#include "Utils/TimeUtils.h"
#include "Global.h"
#include "Utils/StringUtils.h"

static const uint8_t days_in_month[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

//static const char* week_days[7] = { "Sun", "Mon", "Tue", "Wed", "Thr", "Fri", "Sat" };

// String getTimeUnix() {
//     time_t t;
//     struct tm* tm;

//     t = time(NULL);
//     tm = localtime(&t);
//     Serial.printf("%04d/%02d/%02d(%s) %02d:%02d:%02d\n",
//                   tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, week_days[tm->tm_wday],
//                   tm->tm_hour, tm->tm_min, tm->tm_sec);
//     delay(1000);
//     time_t now = time(nullptr);
//     if (now < 30000) {
//         return "failed";
//     }
//     return String(now);
// }

// String getTime() {
//     time_t now = time(nullptr);
//     int zone = 3600 * jsonReadStr(configSetupJson, "timezone").toInt();
//     now = now + zone;
//     String Time = "";                     // Строка для результатов времени
//     Time += ctime(&now);                  // Преобразуем время в строку формата Thu Jan 19 00:55:35 2017
//     int i = Time.indexOf(":");            //Ишем позицию первого символа :
//     Time = Time.substring(i - 2, i + 6);  // Выделяем из строки 2 символа перед символом : и 6 символов после
//     return Time;                          // Возврашаем полученное время
// }

// String getTimeWOsec() {
//     time_t now = time(nullptr);
//     int zone = 3600 * jsonReadStr(configSetupJson, "timezone").toInt();
//     now = now + zone;
//     String Time = "";                     // Строка для результатов времени
//     Time += ctime(&now);                  // Преобразуем время в строку формата Thu Jan 19 00:55:35 2017
//     int i = Time.indexOf(":");            //Ишем позицию первого символа :
//     Time = Time.substring(i - 2, i + 3);  // Выделяем из строки 2 символа перед символом : и 6 символов после
//     return Time;                          // Возврашаем полученное время
// }

// String getDate() {
//     time_t now = time(nullptr);
//     int zone = 3600 * jsonReadStr(configSetupJson, "timezone").toInt();
//     now = now + zone;
//     String Data = "";     // Строка для результатов времени
//     Data += ctime(&now);  // Преобразуем время в строку формата Thu Jan 19 00:55:35 2017
//     Data.replace("\n", "");
//     uint8_t i = Data.lastIndexOf(" ");           //Ишем позицию последнего символа пробел
//     String Time = Data.substring(i - 8, i + 1);  // Выделяем время и пробел
//     Data.replace(Time, "");                      // Удаляем из строки 8 символов времени и пробел
//     return Data;                                 // Возврашаем полученную дату
// }

// String getDateDigitalFormated() {
//     String date = getDate();

//     date = deleteBeforeDelimiter(date, " ");

//     date.replace("Jan", "01");
//     date.replace("Feb", "02");
//     date.replace("Mar", "03");
//     date.replace("Apr", "04");
//     date.replace("May", "05");
//     date.replace("Jun", "06");
//     date.replace("Jul", "07");
//     date.replace("Aug", "08");
//     date.replace("Sep", "09");
//     date.replace("Oct", "10");
//     date.replace("Nov", "11");
//     date.replace("Dec", "12");

//     String month = date.substring(0, 2);
//     String day = date.substring(3, 5);
//     String year = date.substring(8, 10);

//     String out = day;
//     out += ".";
//     out += month;
//     out += ".";
//     out += year;

//     return out;
// }

// int timeToMin(String Time) {
//     //"00:00:00"  время в секунды
//     long min = selectToMarker(Time, ":").toInt() * 60;  //общее количество секунд в полных часах
//     Time = deleteBeforeDelimiter(Time, ":");            // Теперь здесь минуты секунды
//     min += selectToMarker(Time, ":").toInt();           // Добавим секунды из полных минут
//     return min;
// }

static const char* TIME_FORMAT PROGMEM = "%02d:%02d:%02d";
static const char* TIME_FORMAT_WITH_DAYS PROGMEM = "%dd %02d:%02d";

const String prettySeconds(unsigned long time_s) {
    unsigned long tmp = time_s;
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
    }
    else {
        sprintf_P(buf, TIME_FORMAT, hours, minutes, seconds);
    }
    return String(buf);
}

const String prettyMillis(unsigned long time_ms) {
    return prettySeconds(time_ms / 1000);
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
        }
        else {
            result = static_cast<long>((__LONG_MAX__ - finish) + start + 1u);
        }
    }
    else {
        unsigned long passed = start - finish;
        if (passed <= __LONG_MAX__) {
            result = static_cast<long>(passed);
            result = -1 * result;
        }
        else {
            result = static_cast<long>((__LONG_MAX__ - start) + finish + 1u);
            result = -1 * result;
        }
    }
    return result;
}

int getOffsetInSeconds(int timezone) {
    return getOffsetInMinutes(timezone) * ONE_MINUTE_s;
}

int getOffsetInMinutes(int timezone) {
    return timezone * ONE_HOUR_m;
}

void breakEpochToTime(unsigned long epoch, Time_t& tm) {
    // break the given time_input into time components
    // this is a more compact version of the C library localtime function

    unsigned long time = epoch;
    tm.second = time % 60;
    time /= 60;  // now it is minutes
    tm.minute = time % 60;
    time /= 60;  // now it is hours
    tm.hour = time % 24;
    time /= 24;  // now it is days
    tm.days = time;
    tm.day_of_week = ((time + 4) % 7) + 1;  // Sunday is day 1

    uint8_t year = 0;
    unsigned long days = 0;

    while ((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= time) {
        year++;
    }
    tm.year = year - 30;

    days -= LEAP_YEAR(year) ? 366 : 365;
    time -= days;  // now it is days in this year, starting at 0
    tm.day_of_year = time;

    uint8_t month;
    uint8_t month_length;
    for (month = 0; month < 12; month++) {
        if (1 == month) {  // february
            if (LEAP_YEAR(year)) {
                month_length = 29;
            }
            else {
                month_length = 28;
            }
        }
        else {
            month_length = days_in_month[month];
        }

        if (time >= month_length) {
            time -= month_length;
        }
        else {
            break;
        }
    }
    tm.month = month + 1;        // jan is month 1
    tm.day_of_month = time + 1;  // day of month
    tm.valid = (epoch > MIN_DATETIME);
}

void timeInit() {
    ts.add(
        TIME, 1000, [&](void*) {
            String timenow = timeNow->getTimeWOsec();
            static String prevTime;
            if (prevTime != timenow) {
                prevTime = timenow;
                jsonWriteStr(configLiveJson, "timenow", timenow);
                eventGen2("timenow", timenow);
                SerialPrint("I", F("NTP"), timenow);
            }
        },
        nullptr, true);
    SerialPrint("I", F("NTP"), F("Handle time init"));  
}
