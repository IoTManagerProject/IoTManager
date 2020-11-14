#include "items/vInput.h"

#include <Arduino.h>

#include "Class/LineParsing.h"
#include "Global.h"
#include "BufferExecute.h"

Input::Input(String key, String widget) {
    _key = key;
    String value = jsonReadStr(configLiveJson, key);

    if (value == "") {
        if (widget.indexOf("Digit") != -1) {
            value = "52";
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
    jsonWriteStr(configLiveJson, _key, value);
    saveLive();
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

    int number = getKeyNum(key, input_KeyList);

    if (myInput != nullptr) {
        if (number != -1) {
            myInput->at(number).execute(value);
        }
    }
}

