#include "Consts.h"
#ifdef EnableCountDown
#include <Arduino.h>

#include "BufferExecute.h"
#include "Class/LineParsing.h"
#include "Global.h"
#include "items/vCountDown.h"

CountDownClass::CountDownClass(String key) {
    _key = key;
}

CountDownClass::~CountDownClass() {}

void CountDownClass::execute(unsigned int countDownPeriod) {
    _countDownPeriod = countDownPeriod * 1000;
    _start = true;
}

void CountDownClass::loop() {
    if (_countDownPeriod > 0 && _start) {
        prevMillis = millis();
        sec = (_countDownPeriod / 1000);
        _start = false;
    }
    difference = millis() - prevMillis;
    if (difference > 1000 && _countDownPeriod > 0) {
        prevMillis = millis();
        Serial.println(_key + " " + String(sec));
        publishStatus(_key, String(sec));
        sec--;
        if (sec < 0) {
            _countDownPeriod = 0;
            eventGen2(_key, "0");
            Serial.println(_key + " completed");
        }
    }
}

MyCountDownVector* myCountDown = nullptr;

void countDown() {
    myLineParsing.update();
    String key = myLineParsing.gkey();
    myLineParsing.clear();

    countDown_EnterCounter++;
    addKey(key, countDown_KeyList, countDown_EnterCounter);

    Serial.println(countDown_EnterCounter);
    Serial.println(countDown_KeyList);

    static bool firstTime = true;
    if (firstTime) myCountDown = new MyCountDownVector();
    firstTime = false;
    myCountDown->push_back(CountDownClass(key));

    sCmd.addCommand(key.c_str(), countDownExecute);
}

void countDownExecute() {
    String key = sCmd.order();
    String value = sCmd.next();

    if (!isDigitStr(value)) {  //если значение - текст
        value = getValue(value);
    }

    int number = getKeyNum(key, countDown_KeyList);

    if (myCountDown != nullptr) {
        if (number != -1) {
            myCountDown->at(number).execute(value.toInt());
        }
    }
}
#endif