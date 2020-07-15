#include "Clock.h"

#include "Utils/TimeUtils.h"
#include "PrintMessage.h"

#ifdef ESP8266
#include "sntp.h"
#endif

static const char* MODULE = "Clock";

Clock now;

Clock::Clock() : _uptime{millis()},
                 _hasSynced{false},
                 _configured{false} {};

void Clock::setConfig(ClockConfig* config) {
    _config = config;
}

void Clock::loop() {
    unsigned long passed = millis_since(_uptime);
    if (passed < ONE_SECOND_ms) {
        return;
    }
    _uptime += passed;

    if (!_hasSynced) {
        startSync();
    }

    // world time
    time_t now = getSystemTime();
    time_t estimated = _unixtime + (passed / ONE_SECOND_ms);
    double drift = difftime(now, estimated);
    if (drift > 1) {
        // Обработать ситуации c дрифтом времени на значительные величины
    }
    // TODO сохранять время на флеше

    _unixtime = now;

    breakEpochToTime(_unixtime, _time_utc);

    breakEpochToTime(_unixtime + getOffsetInSeconds(_config->getTimezone()), _time_local);
}

void Clock::startSync() {
    if (!_configured) {
        pm.info("ntp:" + _config->getNtp() + ", timezone:" + String(_config->getTimezone()));
        setupSntp();
        _configured = true;
        return;
    }
    _hasSynced = hasSynced();
    if (_hasSynced) {
        pm.info(getDateDotFormated() + " " + getTime());
    }
}

void Clock::setupSntp() {
    configTime(0, 0, _config->getNtp().c_str(), "ru.pool.ntp.org", "pool.ntp.org");
}

bool Clock::hasSynced() const {
    return _unixtime > MIN_DATETIME;
}

time_t Clock::getSystemTime() const {
    timeval tv{0, 0};
    timezone tz = timezone{0, 0};
    time_t epoch = 0;
    if (gettimeofday(&tv, &tz) != -1) {
        epoch = tv.tv_sec;
    }
    return epoch;
}

const unsigned long Clock::getEpoch() {
    return _unixtime;
}

const String Clock::getTimeUnix() {
    return String(_unixtime);
}

/*
    * Локальное время "дд.ММ.гг"
    */
const String Clock::getDateDotFormated() {
    char buf[16];
    sprintf(buf, "%02d.%02d.%02d", _time_local.day_of_month, _time_local.month, _time_local.year);
    return String(buf);
}

/*
    * Локальное дата время "дд.ММ.гг чч.мм.cc"
    */
const String Clock::getDateTimeDotFormated() {
    char buf[32];
    sprintf(buf, "%02d.%02d.%02d %02d:%02d:%02d", _time_local.day_of_month, _time_local.month, _time_local.year, _time_local.hour, _time_local.minute, _time_local.second);
    return String(buf);
}

/*
    * Локальное время "чч:мм:cc"
    */
const String Clock::getTime() {
    char buf[16];
    sprintf(buf, "%02d:%02d:%02d", _time_local.hour, _time_local.minute, _time_local.second);
    return String(buf);
}

const String Clock::getTimeJson() {
    char buf[16];
    sprintf(buf, "%02d-%02d-%02d", _time_local.hour, _time_local.minute, _time_local.second);
    return String(buf);
}

/*
* Локальное время "чч:мм"
*/
const String Clock::getTimeWOsec() {
    char buf[32];
    sprintf(buf, "%02d:%02d", _time_local.hour, _time_local.minute);
    return String(buf);
}

/*
* Время с момента запуска "чч:мм:cc" далее "дд чч:мм"
*/
const String Clock::getUptime() {
    return prettyMillis(_uptime);
}

/*
* Разбивает время на составляющие
*/
void Clock::breakEpochToTime(unsigned long epoch, Time_t& tm) {
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
            } else {
                month_length = 28;
            }
        } else {
            month_length = DAYS_IN_MONTH[month];
        }

        if (time >= month_length) {
            time -= month_length;
        } else {
            break;
        }
    }
    tm.month = month + 1;        // jan is month 1
    tm.day_of_month = time + 1;  // day of month
    tm.valid = (epoch > MIN_DATETIME);
}
