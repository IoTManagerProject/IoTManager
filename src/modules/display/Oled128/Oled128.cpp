#include "Global.h"
#include "classes/IoTItem.h"

#include <GyverOLED.h>

GyverOLED<SSD1306_128x64, OLED_BUFFER> oled;

// GyverOLED<SSD1306_128x32, OLED_BUFFER> oled;
// GyverOLED<SSD1306_128x32, OLED_NO_BUFFER> oled;
// GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> oled;
// GyverOLED<SSD1306_128x64, OLED_BUFFER, OLED_SPI, 8, 7, 6> oled;
// GyverOLED<SSH1106_128x64> oled;

class Oled128 : public IoTItem {
   private:
    unsigned int _x;
    unsigned int _y;

    unsigned int _k;

    int _shrift;

    String _id2show;
    String _descr;
    String _descr1;

    int _prevStrSize;

    bool _isShow = true;  // экран показывает

   public:
    Oled128(String parameters) : IoTItem(parameters) {
        String addr, size, xy, k;
        _prevStrSize = 0;

        jsonRead(parameters, "addr", addr);
        if (addr == "") {
            scanI2C();
            return;
        }

        jsonRead(parameters, "coord", xy);
        _x = selectFromMarkerToMarker(xy, ",", 0).toInt();
        _y = selectFromMarkerToMarker(xy, ",", 1).toInt();

        jsonRead(parameters, "descr", _descr);
        jsonRead(parameters, "id2show", _id2show);
        jsonRead(parameters, "descr1", _descr1);
        // jsonRead(parameters, "scale", _k);
        jsonRead(parameters, "shrift", _shrift);

        // Wire.begin(2,0);       // Инициализация шины I2C для модуля E01

        oled.init();  // инициализация экрана
    }

    void doByInterval() {
        printBlankStr(_prevStrSize);

        String tmpStr = "";

        // if (_descr != "none") tmpStr = _descr + " " + getItemValue(_id2show);
        if (_descr != "none")
            tmpStr = _descr + " " + getItemValue(_id2show) + " " + _descr1;
        else
            tmpStr = getItemValue(_id2show);

        // oled.setScale(2);

        oled.setScale(_shrift);

        oled.setCursorXY(_x, _y);

        oled.print(tmpStr);

        oled.update();

        _prevStrSize = tmpStr.length();
    }

    IoTValue execute(String command, std::vector<IoTValue> &param) {  // будет возможным использовать, когда сценарии запустятся

        if (command == "scroll") {
            String tmpStr = "";
            oled.clear();
            uint32_t tmr = millis();
            oled.autoPrintln(false);
            int val = 128;
            for (;;) {
                // oled.clear();   // ЗАКОММЕНТИРУЙ, ЕСЛИ ВКЛЮЧЕН БУФЕР
                // oled.setScale(2);

                oled.setScale(_shrift);

                oled.setCursor(val, _y);

                oled.print(tmpStr);
                oled.update();
                val--;
                if (millis() - tmr > 5000)
                    ;  // return;

                _isShow = true;
            }

        }

        else if (command == "stopscroll") {
            _isShow = true;
            //    display->backlight();
            // else if (command == "noDisplay") {
            //     display->noDisplay();
            //     _isShow = false;
        } else if (command == "display") {
            // display.display();
            _isShow = true;
        } else if (command == "toggle") {
            if (_isShow) {
                //  display->noDisplay();
                _isShow = false;
            } else {
                //  display.display();
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
        } else if (command == "descr") {
            if (param.size()) {
                _descr = param[0].valS;
            }
        } else if (command == "descr1") {
            if (param.size()) {
                _descr1 = param[0].valS;
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

        //   oled.setScale(2);

        oled.setScale(_shrift);

        oled.setCursorXY(_x, _y);

        oled.print(tmpStr);
    }

    ~Oled128(){};
};

void *getAPI_Oled128(String subtype, String param) {
    if (subtype == F("Oled128")) {
        return new Oled128(param);
    } else {
        return nullptr;
    }
}