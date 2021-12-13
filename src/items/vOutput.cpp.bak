#include "Consts.h"
#ifdef EnableOutput
#include <Arduino.h>

#include "BufferExecute.h"
#include "Class/LineParsing.h"
#include "Clock.h"
#include "Global.h"
#include "items/vOutput.h"

Output::Output(String key) {
    _key = key;
    String value = jsonReadStr(configLiveJson, key);
    this->execute(value);
}
Output::~Output() {}

void Output::execute(String value) {
    eventGen2(_key, value);
    jsonWriteStr(configLiveJson, _key, value);
    publishStatus(_key, value);
    //publishLastUpdateTime(_key, timeNow->getTime());
}

MyOutputVector* myOutput = nullptr;

void outputValue() {
    myLineParsing.update();
    String key = myLineParsing.gkey();
    myLineParsing.clear();

    output_EnterCounter++;
    addKey(key, output_KeyList, output_EnterCounter);

    static bool firstTime = true;
    if (firstTime) myOutput = new MyOutputVector();
    firstTime = false;
    myOutput->push_back(Output(key));

    sCmd.addCommand(key.c_str(), outputExecute);
}

void outputExecute() {
    String key = sCmd.order();
    String value = sCmd.next();

    value.replace("#", " ");
    value.replace("%date%", timeNow->getDateTimeDotFormated());

    int number = getKeyNum(key, output_KeyList);

    if (myOutput != nullptr) {
        if (number != -1) {
            myOutput->at(number).execute(value);
        }
    }
}
#endif
