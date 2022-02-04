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
    String path = mqttRootDevice + "/" + _key + "/status";
    String json = "{}";
    jsonWriteStr(json, "status", value);
    String MyJson = json; 
    jsonWriteStr(MyJson, "topic", path); 
    ws.textAll(MyJson);
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
    String value = ExecuteParser();

    value.replace("#", " ");
    value.replace("%date%", timeNow->getDateTimeDotFormated());
    value.replace("%weekday%", timeNow->getWeekday());
    value.replace("%IP%", jsonReadStr(configSetupJson, F("ip")));
    value.replace("%name%", jsonReadStr(configSetupJson, F("name")));

    int number = getKeyNum(key, output_KeyList);

    if (myOutput != nullptr) {
        if (number != -1) {
            myOutput->at(number).execute(value);
        }
    }
}
#endif
