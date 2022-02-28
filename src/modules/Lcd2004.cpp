#include "Global.h"
#include "classes/IoTItem.h"

#include "LiquidCrystal_I2C.h"
#include <map>

LiquidCrystal_I2C *LCDI2C;

class Lcd2004 : public IoTItem {
   private:
    unsigned int _x;
    unsigned int _y;
    String _id2show;
    String _descr;
    int _prevStrSize;

   public:
    Lcd2004(String parameters) : IoTItem(parameters) {
        String addr, size, xy;
        _prevStrSize = 0;

        jsonRead(parameters, "addr", addr);
        jsonRead(parameters, "size", size);
        int w = selectFromMarkerToMarker(size, ",", 0).toInt();  //количество столбцов
        int h = selectFromMarkerToMarker(size, ",", 1).toInt();  //количество строк
        if (LCDI2C == nullptr) {                                 //инициализации экрана еще не было
            LCDI2C = new LiquidCrystal_I2C(hexStringToUint8(addr), w, h);
            if (LCDI2C != nullptr) {
                LCDI2C->init();
                LCDI2C->backlight();
            }
        }

        jsonRead(parameters, "coord", xy);
        _x = selectFromMarkerToMarker(xy, ",", 0).toInt();
        _y = selectFromMarkerToMarker(xy, ",", 1).toInt();

        jsonRead(parameters, "descr", _descr);
        jsonRead(parameters, "id2show", _id2show);
    }

    void doByInterval() {
        if (LCDI2C != nullptr) {
            printBlankStr(_prevStrSize);

            String tmpStr;
            // to do
            //  jsonRead(paramsHeapJson, _id2show, tmpStr);
            if (_descr != "none") tmpStr = _descr + " " + tmpStr;
            LCDI2C->setCursor(_x, _y);
            LCDI2C->print(tmpStr);

            // LCDI2C->print("Helloy,Manager 404 !");

            _prevStrSize = tmpStr.length();
        }
    }

    IoTValue execute(String command, std::vector<IoTValue> &param) {  // будет возможным использовать, когда сценарии запустятся
        if (command == "noBacklight")
            LCDI2C->noBacklight();
        else if (command == "backlight")
            LCDI2C->backlight();
        else if (command == "noDisplay")
            LCDI2C->noDisplay();
        else if (command == "display")
            LCDI2C->display();
        else if (command == "x") {
            _x = param[0].valD;
        } else if (command == "y") {
            _y = param[0].valD;
        } else if (command == "descr") {
            _descr = param[0].valS;
        } else if (command == "id2show") {
            _id2show = param[0].valS;
        }

        doByInterval();
        return {};
    }

    //печать пустой строки нужной длинны для затирания предыдущего значения на экране
    void printBlankStr(int strSize) {
        String tmpStr = "";
        for (int i = 0; i < strSize; i++) tmpStr += " ";
        LCDI2C->setCursor(_x, _y);
        LCDI2C->print(tmpStr);
    }

    ~Lcd2004(){};
};

void *getAPI_Lcd2004(String subtype, String param) {
    if (subtype == F("Lcd2004")) {
        return new Lcd2004(param);
    } else {
        return nullptr;
    }
}
