#include "Global.h"
#include "classes/IoTItem.h"

#include <TM1637.h>
#include <TM1638.h>
#include <TM16xxDisplay.h>
#include <TM16xxbuttons.h>


TM16xxButtons* buttons = nullptr; // указатель на объект управления кнопками для TM1638 иначе nullptr
IoTItem* iotItemObj = nullptr; // указатель на объект конфигурации для доступа из функций calback

void setIotItemValue(int buttonNum, int state) {
    if (iotItemObj) {
        String id = iotItemObj->getID() + "_" + String(buttonNum);
        for (std::list<IoTItem*>::iterator it = IoTItems.begin(); it != IoTItems.end(); ++it) {
            if ((*it)->getID() == id) {
                IoTValue value;
                value.valD = state;
                (*it)->setValue(value);
                //value.valD = 0;  // сбрасываем состояние в нулевое если статус конечный (т.к. библиотека отрабатывает события не по порядку)
                //if (state == 1 || state == 5 || state == 2) (*it)->setValue(value, false);  
                break;
            }
        }
    }
}

// The Release function will be called when a button was released.
// It can be used for fast actions when no click or double click needs to be detected.
void fnRelease(byte nButton) {
    // using isPressed or is LongPressed a shift-key can be implemented
    if(buttons->isLongPressed(0))
        Serial.print(F("Button 0 still longpressed. "));
    else if(buttons->isPressed(0))
        Serial.print(F("Button 0 still pressed. "));

    Serial.print(F("Button "));
    Serial.print(nButton);
    Serial.println(F(" release."));

    setIotItemValue(nButton, 0);
} // release


// This function will be called when a button was pressed 1 time (without a second press).
void fnClick(byte nButton) {
    Serial.print(F("Button "));
    Serial.print(nButton);
    Serial.println(F(" click."));

    setIotItemValue(nButton, 1);
} // click


// This function will be called when a button was pressed 2 times in a short timeframe.
void fnDoubleclick(byte nButton) {
    Serial.print(F("Button "));
    Serial.print(nButton);
    Serial.println(F(" doubleclick."));

    setIotItemValue(nButton, 2);
} // doubleclick


// This function will be called once, when a button is pressed for a long time.
void fnLongPressStart(byte nButton) {
    Serial.print(F("Button "));
    Serial.print(nButton);
    Serial.println(F(" longPress start"));

    setIotItemValue(nButton, 3);
} // longPressStart


// This function will be called often, while a button is pressed for a long time.
void fnLongPress(byte nButton) {
    Serial.print(F("Button "));
    Serial.print(nButton);
    Serial.println(F(" longPress..."));

    setIotItemValue(nButton, 4);
} // longPress


// This function will be called once, when a button is released after beeing pressed for a long time.
void fnLongPressStop(byte nButton) {
    Serial.print(F("Button "));
    Serial.print(nButton);
    Serial.println(F(" longPress stop"));

    setIotItemValue(nButton, 5);
} // longPressStop


class TM16XX : public IoTItem {
   private:
    TM16xxDisplay *_display = nullptr;
    TM16xx *_module = nullptr;
    String _id2show;

   public:
    TM16XX(String parameters) : IoTItem(parameters) {
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

        jsonRead(parameters, "id2show", _id2show);

        if (chip == 1637) {
            _module = new TM1637(DIO, CLK, numDigits);
        } else if (chip == 1638) {
            _module = new TM1638(DIO, CLK, STB, numDigits);
            buttons = new TM16xxButtons(_module);
            buttons->attachRelease(fnRelease);
            buttons->attachClick(fnClick);
            buttons->attachDoubleClick(fnDoubleclick);
            buttons->attachLongPressStart(fnLongPressStart);
            buttons->attachLongPressStop(fnLongPressStop);
            buttons->attachDuringLongPress(fnLongPress);
        }
        _module->setupDisplay(onoff, intensity);
        _display = new TM16xxDisplay(_module, numDigits);
        _display->println(getValue());
    }

    byte btLeds=0;
    byte btPosition=0;
    void loop() {
        if(buttons) buttons->tick();
    }

    void setValue(const IoTValue& Value, bool genEvent = true) {
        if (_display == nullptr) return;

        value = Value;
        //_display->println(getValue());
        _display->printf("%4s\n", getValue());
        IoTItem::setValue(Value, genEvent);
    }

    void onRegEvent(IoTItem* eventItem) {
        if (_display == nullptr) return;
        if (!eventItem || _id2show == "") return; 
     
        //_display->println(eventItem->getValue());
        if (_id2show == eventItem->getID()) {
            setValue(eventItem->value, false);
        }
        // } else {
        //     _display->println();
        //     for (int i = 0; i < _ids2show.size(); i++) {
        //         IoTItem* item = findIoTItem(_ids2show[i]);
        //         if (item) {
        //             _display->print(item->getValue());    
        //         }
        //     }
        // }
    }

    IoTValue execute(String command, std::vector<IoTValue>& param) {
        if (command == "setLEDs") {
            if (param.size() == 1) {
                ((TM1638*)_module)->setLEDs(param[0].valD);
            }
        } else if (command == "onLED") {
            if (param.size() == 1) {
                ((TM1638*)_module)->setLED(TM1638_COLOR_RED, param[0].valD - 1);
            }
        } else if (command == "offLED") {
            if (param.size() == 1) {
                ((TM1638*)_module)->setLED(TM1638_COLOR_GREEN, param[0].valD - 1);
            }
        } else if (command == "setParamLED") {
            if (param.size() == 2) {
                _module->setupDisplay(param[0].valD, param[1].valD);
            }
        } else if (command == "id2show") {
            if (param.size()  == 1) {
                _id2show = param[0].valS;
            }
        }

        return {};
    }

    ~TM16XX() {
        if (_display) delete _display;
        if (_module) delete _module;
        if (buttons) delete buttons;
        iotItemObj = nullptr;
        buttons = nullptr;
    };
};

void *getAPI_TM16XX(String subtype, String param) {
    if (subtype == F("TM16XX")) {
        return iotItemObj = new TM16XX(param);
    } else {
        return nullptr;
    }
}
