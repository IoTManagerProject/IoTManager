#include "Consts.h"
#ifdef EnableInput
#include "items/vInput.h"

#include <Arduino.h>

#include "BufferExecute.h"
#include "Class/LineParsing.h"
#include "Clock.h"
#include "Global.h"

//this class save date to flash
Input::Input(String key, String widget) {
    _key = key;
    String value = jsonReadStr(configStoreJson, key);

    if (value == "") {
        if (widget.indexOf("Digit") != -1) {
            value = "25";
        }
        if (widget.indexOf("Time") != -1) {
            value = "12:00";
        }
    }

    this->execute(value);
}
Input::~Input() {}

void Input::execute(String value) {
    eventGen2(_key, value);
    jsonWriteStr(configStoreJson, _key, value);
    saveStore();
    publishStatus(_key, value);
}

MyInputVector* myInput = nullptr;

void input() {
    myLineParsing.update();
    String widget = myLineParsing.gfile();
    String key = myLineParsing.gkey();
    myLineParsing.clear();

    input_EnterCounter++;
    addKey(key, input_KeyList, input_EnterCounter);

    static bool firstTime = true;
    if (firstTime) myInput = new MyInputVector();
    firstTime = false;
    myInput->push_back(Input(key, widget));

    sCmd.addCommand(key.c_str(), inputExecute);
}

void inputExecute() {
    String key = sCmd.order();
    String value = sCmd.next();

    if (!isDigitStr(value)) {            //если значение - текст
        if (value.indexOf(":") == -1) {  //если этот текст не время
            String valueJson = getValue(value);
            if (valueJson != "no value") {  //если это ключ переменной
                value = valueJson;
            } else {  //если это просто текст
                value.replace("#", " ");
                value.replace("%date%", timeNow->getDateTimeDotFormated());
            }
        }
    }

    int number = getKeyNum(key, input_KeyList);

    if (myInput != nullptr) {
        if (number != -1) {
            myInput->at(number).execute(value);
        }
    }
}
#endif
