#include "Global.h"
#include "classes/IoTItem.h"
#include <iarduino_RTC.h>

class RTC : public IoTItem {
   private:
    bool _ticker = false;
    iarduino_RTC* _watch;
    String _timeFormat = "";

   public:
    RTC(String parameters) : IoTItem(parameters) {
        jsonRead(parameters, F("ticker"), _ticker);
        jsonRead(parameters, F("timeFormat"), _timeFormat);
        _timeFormat = _timeFormat + " ";    // костыль для коррекции ошибки в библиотеке
        
        int chipCode = 3;
        jsonRead(parameters, F("chipCode"), chipCode);
        if (chipCode == 1) {
            int RST, CLK, DAT;
            jsonRead(parameters, "RST", RST);
            jsonRead(parameters, "CLK", CLK);
            jsonRead(parameters, "DAT", DAT);
            _watch = new iarduino_RTC(RTC_DS1302, RST, CLK, DAT);
        } else {
            _watch = new iarduino_RTC(chipCode);
        }
        _watch->begin();
    }

    void doByInterval() {
        value.isDecimal = false;
        value.valS = _watch->gettime(_timeFormat);

        if (_ticker) regEvent(value.valS, F("RTC tick"));
    }

    IoTItem* getRtcDriver() {
        return this;
    }

    unsigned long getRtcUnixTime() {
        return _watch->gettimeUnix() - jsonReadInt(settingsFlashJson, F("timezone")) * 60 * 60;
    }

    void onModuleOrder(String &key, String &value) {
        if (key == "setUTime") {
            char *stopstring;
            unsigned long ut = strtoul(value.c_str(), &stopstring, 10);
            _watch->settimeUnix(ut);
            SerialPrint("i", F("RTC"), "Устанавливаем время: " + value);
        } else if (key == "setSysTime") {
            _watch->settimeUnix(unixTime + jsonReadInt(settingsFlashJson, F("timezone")) * 60 * 60);
            SerialPrint("i", F("RTC"), F("Запоминаем системное время"));
        }
    }

    IoTValue execute(String command, std::vector<IoTValue> &param) {
        if (command == "getTime") { 
            if (param.size() == 1) {
                IoTValue valTmp;
                valTmp.isDecimal = false;
                valTmp.valS = _watch->gettime(param[0].valS + " ");
                return valTmp;
            }
        } else if (command == "setUnixTime") {
            if (param.size() == 1) {
                long ut = strtoul(param[0].valS.c_str(), nullptr, 10);
                _watch->settimeUnix(ut);
                return {};
            }
        } else if (command == "setTime") {
            if (param.size() == 6) {
                _watch->settime(param[0].valD, param[1].valD, param[2].valD, param[3].valD, param[4].valD, param[5].valD);      //сек, мин, час, день, мес, год
                return {};
            }
        } else if (command == "getTimeFloat") {
            if (param.size() == 1) {
                IoTValue valTmp;
                _watch->gettime();
                valTmp.isDecimal = true;
                String type = param[0].valS;
                    if (type == "H") {
                        valTmp.valD = static_cast<float>(_watch->Hours);
                    } else if (type == "i") {
                        valTmp.valD = static_cast<float>(_watch->minutes);
                    } else if (type == "s") {
                        valTmp.valD = static_cast<float>(_watch->seconds);
                    } else if (type == "w") {
                        valTmp.valD = static_cast<float>(_watch->weekday);                        
                    } else if (type == "d") {
                        valTmp.valD = static_cast<float>(_watch->day);      
                    } else if (type == "m") {
                        valTmp.valD = static_cast<float>(_watch->month);
                    } else if (type == "Y") {
                        valTmp.valD = static_cast<float>(_watch->year);                                           
                    } else {
                        return {};  // Если переданный тип не поддерживается
                    }
                return valTmp;
            }
        }

        return {}; 
    }

    ~RTC(){
        if (_watch) delete _watch;
    };
};

void* getAPI_RTC(String subtype, String param) {
    if (subtype == F("RTC")) {
        return new RTC(param);
    } else {
        return nullptr;
    }
}
