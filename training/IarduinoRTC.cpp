//#include "Global.h"
//#include "classes/IoTItem.h"
//
//#include <iarduino_RTC.h>
//#include "iarduino_RTC_DS1302.h"  //	Подключаем файл iarduino_RTC_DS1302.h
//#include "iarduino_RTC_DS1307.h"  //	Подключаем файл iarduino_RTC_DS1307.h
//#include "iarduino_RTC_DS3231.h"  //	Подключаем файл iarduino_RTC_DS3231.h
//#include "iarduino_RTC_RX8025.h"
//
// class IarduinoRTC : public IoTItem {
//   private:
//    int _chipNum, _rst, _clk, _dat, _ticker;
//    String _defFormat;
//    iarduino_RTC_BASE* RTCDriver;
//
//   public:
//    IarduinoRTC(String parameters) : IoTItem(parameters) {
//        jsonRead(parameters, "chipNum", _chipNum);
//        jsonRead(parameters, "rst", _rst);
//        jsonRead(parameters, "clk", _clk);
//        jsonRead(parameters, "dat", _dat);
//        jsonRead(parameters, "defFormat", _defFormat);
//        jsonRead(parameters, "ticker", _ticker);
//
//        switch (_chipNum) {
//            case 0:
//                RTCDriver = new iarduino_RTC_NTP();
//                break;
//            case 1:
//                RTCDriver = new iarduino_RTC_DS1302(_rst, _clk, _dat);
//                break;
//            case 2:
//                RTCDriver = new iarduino_RTC_DS1307();
//                break;
//            case 3:
//                RTCDriver = new iarduino_RTC_DS3231();
//                break;
//            case 4:
//                RTCDriver = new iarduino_RTC_RX8025();
//                break;
//        }
//        if (RTCDriver) RTCDriver->begin();
//    }
//
//    void doByInterval() {
//        value.isDecimal = false;
//        value.valS = watch->gettime(_defFormat.c_str());
//        if (_ticker) regEvent(value.valS, "time ticker");
//    }
//
//    IoTValue execute(String command, std::vector<IoTValue>& param) {
//        IoTValue tmpValue;
//        if (command == "getTime") {
//            if (param.size()) {
//                tmpValue.isDecimal = false;
//                tmpValue.valS = watch->gettime(param[0].valS.c_str());
//                return tmpValue;
//            }
//        } else if (command == "saveSysTime") {
//            tm localTimeVar;
//            time_t timeNowVar;
//            time(&timeNowVar);
//            localTimeVar = *localtime(&timeNowVar);
//            watch->settime(localTimeVar.tm_sec, localTimeVar.tm_min, localTimeVar.tm_hour, localTimeVar.tm_mday, localTimeVar.tm_mon + 1, localTimeVar.tm_year - 100, localTimeVar.tm_wday);
//        } else if (command == "setTime") {
//            if (param.size()) {
//                watch->settime(param[0].valD, param[1].valD, param[2].valD, param[3].valD, param[4].valD, param[5].valD, param[6].valD);
//            }
//        } else if (command == "setUnixTime") {
//            if (param.size()) {
//                watch->settimeUnix(param[0].valD);
//            }
//        } else if (command == "getHours") {
//            tmpValue.valD = watch->Hours;  //	Часы			0-23
//            return tmpValue;
//        } else if (command == "getMinutes") {
//            tmpValue.valD = watch->minutes;  //	Минуты			0-59
//            return tmpValue;
//        } else if (command == "getSeconds") {
//            tmpValue.valD = watch->seconds;  //	Секунды			0-59
//            return tmpValue;
//        } else if (command == "getMonth") {
//            tmpValue.valD = watch->month;  //	Месяц			1-12
//            return tmpValue;
//        } else if (command == "getDay") {
//            tmpValue.valD = watch->day;  //	День месяца		1-31
//            return tmpValue;
//        }
//
//        return {};
//    }
//
//    iarduino_RTC_BASE* getRtcDriver() {
//        return RTCDriver;
//    }
//
//    ~IarduinoRTC(){};
//};
//
// void* getAPI_IarduinoRTC(String subtype, String param) {
//    if (subtype == F("IarduinoRTC")) {
//        return new IarduinoRTC(param);
//    } else {
//        return nullptr;
//    }
//}
