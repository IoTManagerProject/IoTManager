#include "Global.h"
#include "classes/IoTItem.h"

#include <OneWire.h>

bool cmpAddr(uint8_t *addr1, uint8_t *addr2, int size) {
    for (int i=0; i<size; i++) {
        if (addr1[i] != addr2[i]) return false;
    }
    return true;
}

class DS2401 : public IoTItem {
   private:
    OneWire* _ds = nullptr;
    byte _addr[8], _oldAddr[8];
    char _addrStr[20];
    int _intRepeat;
    int _repeatCount = 0;

   public:
    DS2401(String parameters): IoTItem(parameters) {
        int pin;
        jsonRead(parameters, "pin", pin);
        jsonRead(parameters, "intRepeat", _intRepeat);
        _interval = _interval / 1000;   // корректируем величину интервала int, теперь он в миллисекундах
        _ds = new OneWire(pin);
    }

    void doByInterval() {
        if (_ds->search(_addr) && _ds) {
            if ( OneWire::crc8( _addr, 7) != _addr[7]) {
                SerialPrint("E", "DS2401", "CRC is not valid!", _id);
                return;
            }

            if (!cmpAddr(_addr, _oldAddr, 8) || _repeatCount > _intRepeat) {                
                hex2string(_addr, 8, _addrStr);
                value.valS = _addrStr;
                value.isDecimal = false;
                regEvent(value.valS, "DS2401");
                memcpy(_oldAddr, _addr, 8);
                _repeatCount = 0;
            }
        }

        if (_repeatCount <= _intRepeat) _repeatCount++;
    }

    ~DS2401() {
        if (_ds) delete _ds;
    };
};

void* getAPI_DS2401(String subtype, String param) {
    if (subtype == F("DS2401")) {
        return new DS2401(param);
    } else {
        return nullptr;  
    }
}

