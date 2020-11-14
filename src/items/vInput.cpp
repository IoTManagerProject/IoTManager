#include "items/vInput.h"

#include <Arduino.h>

#include "Class/LineParsing.h"
#include "Global.h"
#include "BufferExecute.h"

Input::Input(String key) {
    _key = key;
    String value = jsonReadStr(configLiveJson, key);
    this->execute(value);
}
Input::~Input() {}

void Input::execute(String state) {
    eventGen2(_key, state);
    jsonWriteInt(configLiveJson, _key, state.toInt());
    saveLive();
    publishStatus(_key, state);
}

MyInputVector* myInput = nullptr;

void input() {
    myLineParsing.update();
    String key = myLineParsing.gkey();
    myLineParsing.clear();

    input_EnterCounter++;
    addKey(key, input_KeyList, input_EnterCounter);

    static bool firstTime = true;
    if (firstTime) myInput = new MyInputVector();
    firstTime = false;
    myInput->push_back(Input(key));

    sCmd.addCommand(key.c_str(), inputExecute);
}

void inputExecute() {
    String key = sCmd.order();
    String state = sCmd.next();

    int number = getKeyNum(key, input_KeyList);

    if (myInput != nullptr) {
        if (number != -1) {
            myInput->at(number).execute(state);
        }
    }
}

