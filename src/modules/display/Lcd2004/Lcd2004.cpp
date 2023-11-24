#include "Global.h"
#include "classes/IoTItem.h"
#include <RobotClass_LiquidCrystal_I2C.h>

RobotClass_LiquidCrystal_I2C* LCDI2C;

class Lcd2004 : public IoTItem {
   private:
    unsigned int _x;
    unsigned int _y;
    String _id2show, _prefix = "", _postfix = "";
    int _prevStrSize;
    String _addr;

    bool _isShow = true;  // экран показывает

   public:
    Lcd2004(String parameters) : IoTItem(parameters) {
        String size, xy;
        _prevStrSize = 0;

        jsonRead(parameters, "addr", _addr);
        if (_addr == "") {
            scanI2C();
            return;
        }

        jsonRead(parameters, "size", size);
        int w = selectFromMarkerToMarker(size, ",", 0).toInt();  // количество столбцов
        int h = selectFromMarkerToMarker(size, ",", 1).toInt();  // количество строк
        if (LCDI2C == nullptr) {                                 // инициализации экрана еще не было
            LCDI2C = new RobotClass_LiquidCrystal_I2C(hexStringToUint8(_addr), w, h, CP_UTF8);
            if (LCDI2C != nullptr) {
                LCDI2C->init();
                LCDI2C->clear();
                LCDI2C->backlight();
            }
        }

        jsonRead(parameters, "coord", xy);
        _x = selectFromMarkerToMarker(xy, ",", 0).toInt();
        _y = selectFromMarkerToMarker(xy, ",", 1).toInt();

        jsonRead(parameters, "id2show", _id2show);
        jsonRead(parameters, "prefix", _prefix);
        jsonRead(parameters, "postfix", _postfix);
    }

    void drawItem(IoTItem* item) {
        String tmpStr = _prefix;
        tmpStr += item->getValue();
        tmpStr += _postfix;

        printBlankStr(_prevStrSize);
        LCDI2C->setCursor(_x, _y);
        LCDI2C->print(tmpStr);
        _prevStrSize = tmpStr.length();
    }

    void setValue(const IoTValue& Value, bool genEvent = true) {
        if (LCDI2C == nullptr) return;

        value = Value;
        drawItem(this);
        IoTItem::setValue(Value, genEvent);
    }

    void onRegEvent(IoTItem* eventItem) {
        if (LCDI2C == nullptr) {
            scanI2C();
            return;
        }
        if (!eventItem || _id2show == "") return;

        if (_id2show == eventItem->getID()) {
            setValue(eventItem->value, false);
        }
    }

    IoTValue execute(String command, std::vector<IoTValue>& param) {
        if (command == "noBacklight")
            LCDI2C->noBacklight();
        else if (command == "backlight")
            LCDI2C->backlight();
        else if (command == "noDisplay") {
            LCDI2C->noDisplay();
            _isShow = false;
        } else if (command == "display") {
            LCDI2C->display();
            _isShow = true;
        } else if (command == "toggle") {
            if (_isShow) {
                LCDI2C->noDisplay();
                _isShow = false;
            } else {
                LCDI2C->display();
                _isShow = true;
            }
        } else if (command == "x") {
            if (param.size()) {
                _x = param[0].valD;
            }
        } else if (command == "y") {
            if (param.size()) {
                _y = param[0].valD;
            }
        } else if (command == "prefix") {
            if (param.size()) {
                _prefix = param[0].valS;
            }
        } else if (command == "postfix") {
            if (param.size()) {
                _postfix = param[0].valS;
            }
        } else if (command == "id2show") {
            if (param.size()) {
                _id2show = param[0].valS;
            }
        }

        doByInterval();
        return {};
    }

    // печать пустой строки нужной длинны для затирания предыдущего значения на экране
    void printBlankStr(int strSize) {
        String tmpStr = "";
        for (int i = 0; i < strSize; i++) tmpStr += " ";
        LCDI2C->setCursor(_x, _y);
        LCDI2C->print(tmpStr);
    }

    ~Lcd2004() {
        if (LCDI2C) delete LCDI2C;
        LCDI2C = nullptr;
    };
};

void* getAPI_Lcd2004(String subtype, String param) {
    if (subtype == F("Lcd2004")) {
        return new Lcd2004(param);
    } else {
        return nullptr;
    }
}
