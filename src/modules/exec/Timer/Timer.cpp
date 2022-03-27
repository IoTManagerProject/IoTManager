#include "Global.h"
#include "classes/IoTItem.h"

extern IoTGpio IoTgpio;


class Timer : public IoTItem {
   private:
    bool _unfin = false;
    bool _ticker = false;
    bool _repeat = false;
    bool _needSave = false;

   public:
    Timer(String parameters): IoTItem(parameters) {
        float valDtmp;
        jsonRead(parameters, "countDown", valDtmp);
        if (value.valD == 0) value.valD = valDtmp;
        jsonRead(parameters, "ticker", _ticker);
        jsonRead(parameters, "repeat", _repeat);
        if (_repeat) _repeat = valDtmp;  // если в параметрах просят повторить, то запоминаем настроечное значение отчета
        _unfin = !value.valD;
        jsonRead(parameters, "needSave", _needSave);    // нужно сохранять счетчик в постоянную память
    }

    void doByInterval() {
        if (!_unfin && value.valD) {
            value.valD--;
            if (_needSave) needSave = true;
            if (value.valD == 0) {
                regEvent(value.valD, "Time's up");
                if (_repeat) value.valD = _repeat;
            }
        }

        if (_ticker) regEvent(value.valD, "Timer tick");
    }

    ~Timer() {};
};

void* getAPI_Timer(String subtype, String param) {
    if (subtype == F("Timer")) {
        return new Timer(param);
    } else {
        return nullptr;
    }
}
