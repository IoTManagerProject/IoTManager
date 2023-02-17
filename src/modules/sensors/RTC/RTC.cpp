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

    ulong getRtcUnixTime() {
        return _watch->gettimeUnix();
    }

    void onModuleOrder(String &key, String &value) {
        if (key == "setUTime") {
            char *stopstring;
            ulong ut = strtoul(value.c_str(), &stopstring, 10);
            _watch->settimeUnix(ut);
            SerialPrint("i", F("RTC"), "Устанавливаем время: " + value);
        } else if (key == "setSysTime") {
            _watch->settimeUnix(unixTime);
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
