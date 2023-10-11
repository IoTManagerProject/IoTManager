

#include "Global.h"
#include "classes/IoTItem.h"

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 0
Adafruit_SSD1306 display(OLED_RESET);

class Oled64 : public IoTItem {
   private:
    unsigned int _x;
    unsigned int _y;
    String _id2show, _prefix = "", _postfix = "";
    String _size = "1";
    String _addr;

    int _prevStrSize;

    bool _isShow = true;

   public:
    Oled64(String parameters) : IoTItem(parameters) {
        String size, xy;
        _prevStrSize = 0;

        jsonRead(parameters, "addr", _addr);
        if (_addr == "") {
            scanI2C();
            return;
        }

        display.begin(SSD1306_SWITCHCAPVCC, hexStringToUint8(_addr));

        display.display();

        display.clearDisplay();

        jsonRead(parameters, "coord", xy);
        _x = selectFromMarkerToMarker(xy, ",", 0).toInt();
        _y = selectFromMarkerToMarker(xy, ",", 1).toInt();
        jsonRead(parameters, "id2show", _id2show);
        jsonRead(parameters, "prefix", _prefix);
        jsonRead(parameters, "postfix", _postfix);
        jsonRead(parameters, "size", _size);
    }

    void drawItem(IoTItem *item) {
        String tmpStr = _prefix;
        tmpStr += item->getValue();
        tmpStr += _postfix;

        display.setRotation(0);
        display.setCursor(_x, _y);
        display.setTextColor(WHITE, BLACK);
        display.setTextSize(_size.toInt());

        printBlankStr(_prevStrSize);

        display.setCursor(_x, _y);

        display.print(tmpStr);
        _prevStrSize = tmpStr.length();
        display.display();
        _prevStrSize = tmpStr.length();
    }

    void setValue(const IoTValue &Value, bool genEvent = true) {
        value = Value;
        drawItem(this);
        IoTItem::setValue(Value, genEvent);
    }

    void onRegEvent(IoTItem *eventItem) {
        if (!eventItem || _id2show == "") return;

        if (_id2show == eventItem->getID()) {
            setValue(eventItem->value, false);
        }
    }

    IoTValue execute(String command, std::vector<IoTValue> &param) {
        if (command == "display") {
            _isShow = true;
        } else if (command == "noDisplay") {
            _isShow = false;
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
        display.setCursor(_x, _y);
        display.print(tmpStr);
    }

    ~Oled64(){};
};

void *getAPI_Oled64(String subtype, String param) {
    if (subtype == F("Oled64")) {
        return new Oled64(param);
    } else {
        return nullptr;
    }
}