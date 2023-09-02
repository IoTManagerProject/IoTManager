#include "Global.h"
#include "classes/IoTItem.h"

#include <TM1637.h>
#include <TM1638.h>
#include <TM16xxDisplay.h>



class TM16XX : public IoTItem {
   private:
    TM16xxDisplay *_display = nullptr;
    TM16xx *_module = nullptr;
    std::vector<String> _ids2show;

   public:
    TM16XX(String parameters) : IoTItem(parameters) {
        //jsonRead(parameters, "id2show", _id2show);

        int DIO, CLK, STB, chip, numDigits, intensity;
        bool onoff;
        String id2show;
        jsonRead(parameters, "DIO", DIO);
        jsonRead(parameters, "CLK", CLK);
        jsonRead(parameters, "STB", STB);
        jsonRead(parameters, "chip", chip);
        jsonRead(parameters, "numDigits", numDigits);
        jsonRead(parameters, "intensity", intensity);
        jsonRead(parameters, "on", onoff);

        jsonRead(parameters, "id2show", id2show);
        if (id2show != "") _ids2show = splitStr(id2show, ",");

        if (chip == 1637) {
            _module = new TM1637(DIO, CLK, numDigits);
        } else if (chip == 1638) {
            _module = new TM1638(DIO, CLK, STB, numDigits);
        }
        _module->setupDisplay(onoff, intensity);
        _display = new TM16xxDisplay(_module, numDigits);
    }

    void doByInterval() {
        
    }

    void setValue(const IoTValue& Value, bool genEvent = true) {
        if (_display == nullptr) return;

        value = Value;
        _display->println(getValue());
        IoTItem::setValue(Value, genEvent);
    }

    void onRegEvent(IoTItem* eventItem) {
        if (_display == nullptr) return;
        if (!eventItem || _ids2show.size() == 0) return; 

        if (strInVector(eventItem->getID(), _ids2show)) {
            if (_ids2show.size() == 1) {
                _display->println(eventItem->getValue());
            } else {
                _display->println();
                for (int i = 0; i < _ids2show.size(); i++) {
                    IoTItem* item = findIoTItem(_ids2show[i]);
                    if (item) {
                        _display->print(item->getValue());    
                    }
                }
            }
        }
    }

    ~TM16XX() {
        delete _display;
        delete _module;
    };
};

void *getAPI_TM16XX(String subtype, String param) {
    if (subtype == F("TM16XX")) {
        return new TM16XX(param);
    } else {
        return nullptr;
    }
}
