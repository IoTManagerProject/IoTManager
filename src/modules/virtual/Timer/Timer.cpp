#include "Global.h"
#include "classes/IoTItem.h"

extern IoTGpio IoTgpio;


class Timer : public IoTItem {
   private:
    bool _unfin = false;
    bool _ticker = false;
    bool _repeat = false;
    bool _needSave = false;
    bool _pause = false; 
    int _initValue;

   public:
    Timer(String parameters): IoTItem(parameters) {
        jsonRead(parameters, "countDown", _initValue);
        _unfin = !_initValue;
        value.valD = _initValue;                    
        if (_initValue) value.valD = value.valD + 1;        // +1 - компенсируем ранний вычет счетчика, ранний вычет, чтоб после события значение таймера не исказилось.     
                                                // +0 - если изначально установили бесконечный счет
        
        jsonRead(parameters, "ticker", _ticker);
        jsonRead(parameters, "repeat", _repeat);
        jsonRead(parameters, "needSave", _needSave);    // нужно сохранять счетчик в постоянную память
    }

    void doByInterval() {
        if (!_unfin && value.valD >= 0 && !_pause) {
            if (_repeat && value.valD == 0) value.valD = _initValue;
            value.valD--;
            if (_needSave) needSave = true;
            if (value.valD == 0) {
                regEvent(value.valD, "Time's up");
            }
        }

        if (_ticker && (value.valD > 0 || _unfin) && !_pause) regEvent(value.valD, "Timer tick");
    }

    IoTValue execute(String command, std::vector<IoTValue> &param) {
        if (command == "stop") { 
            _pause = true;
        } else if (command == "reset") {
            _pause = false;
            value.valD = _initValue;
            if (_initValue) value.valD = value.valD + 1;
        } else if (command == "continue") {
            _pause = false;
        } else if (command == "int") {
            if (param.size()) {
                setInterval(param[0].valD);
            }
        }

        return {}; 
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
