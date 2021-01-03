#include "items/vInOutput.h"
#include <Arduino.h>
#include "Class/LineParsing.h"
#include "Global.h"
#include "BufferExecute.h"
#include "Clock.h"

//this class save date to flash
InOutput::InOutput(String key, String widget) {
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
InOutput::~InOutput() {}

void InOutput::execute(String value) {
    eventGen2(_key, value);
    jsonWriteStr(configStoreJson, _key, value);
    saveStore();
    publishStatus(_key, value);
}

MyInOutputVector* myInOutput = nullptr;

void inOutput() {
    myLineParsing.update();
    String widget = myLineParsing.gfile();
    String key = myLineParsing.gkey();
    myLineParsing.clear();

    inOutput_EnterCounter++;
    addKey(key, inOutput_KeyList, inOutput_EnterCounter);

    static bool firstTime = true;
    if (firstTime) myInOutput = new MyInOutputVector();
    firstTime = false;
    myInOutput->push_back(InOutput(key, widget));

    sCmd.addCommand(key.c_str(), inOutputExecute);
}

void inOutputExecute() {
    String key = sCmd.order();
    String value = sCmd.next();
    //String type = sCmd.next();


    if (!isDigitStr(value)) { //если значение - текст
        if (value.indexOf(":") == -1) { //если этот текст не время
            String valueJson = getValue(value);
            if (valueJson != "no value") {  //если это ключ переменной
                value = valueJson;
            }
            else { //если это просто текст
                value.replace("#", " ");
                value.replace("%date%", timeNow->getDateTimeDotFormated());
            }
        }
    }

    int number = getKeyNum(key, inOutput_KeyList);

    if (myInOutput != nullptr) {
        if (number != -1) {
            myInOutput->at(number).execute(value);
        }
    }
}

