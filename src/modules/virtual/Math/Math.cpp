#include "Global.h"
#include "classes/IoTItem.h"
#include <ctime>

class IoTMath : public IoTItem {
private:

    time_t convertTime(float day, float month, float year, float hour, float minute) {
        // Преобразование из float в int
        int d = static_cast<int>(day);
        int m = static_cast<int>(month);
        int y = static_cast<int>(year);
        int h = static_cast<int>(hour);
        int min = static_cast<int>(minute);

        if (d < 1 || d > 31 || m < 1 || m > 12 || y < 1900 || h < 0 || h > 23 || min < 0 || min > 59) {
            SerialPrint("E", F("IoTMath"), F("Invalid date or time parameters!"));
            return -1;
        }

        // Структура для хранения даты и времени
        struct tm t;
        t.tm_year = y - 1900;
        t.tm_mon = m - 1;
        t.tm_mday = d;
        t.tm_hour = h;
        t.tm_min = min;
        t.tm_sec = 0;
        t.tm_isdst = -1;  // Пусть система сама определяет DST

        return mktime(&t);
    }

public:
    IoTMath(String parameters) : IoTItem(parameters) {}

    IoTValue execute(String command, std::vector<IoTValue> &param) {
        if(command == "map" && param.size() == 5) {
            IoTValue valTmp;
            valTmp.isDecimal = true;
            valTmp.valD = map(param[0].valD, param[1].valD, param[2].valD, param[3].valD, param[4].valD);
            //SerialPrint("i", F("IoTMath"), F("Mapping value done."));
            return valTmp;
        } else if(command == "convertTime" && param.size() == 5) {
            time_t unixTime = convertTime(param[0].valD, param[1].valD, param[2].valD, param[3].valD, param[4].valD);

            if (unixTime == -1) {
                SerialPrint("E", F("IoTMath"), F("Failed to convert time."));
                return {};
            }

            IoTValue valTmp;
            valTmp.isDecimal = true;
            valTmp.valD = unixTime;
            //SerialPrint("i", F("IoTMath"), F("Time conversion done."));
            return valTmp;
        }

        SerialPrint("E", F("IoTMath"), F("Unknown command or wrong parameters."));
        return {};
    }
};

void *getAPI_IoTMath(String subtype, String param) {
    if (subtype == F("IoTMath")) {
        return new IoTMath(param);
    }
    return nullptr;
}
