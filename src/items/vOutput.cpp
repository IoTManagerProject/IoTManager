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
    #ifdef WEBSOCKET_ENABLED
    ws.textAll(MyJson);
    #endif
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

String ExecuteParser() {
 String value = "";   
 String value1 = sCmd.next();
 String value2 = sCmd.next();
 String value3 = sCmd.next();
 if (getValue(value1)  != "no value")
 {value1 = getValue(value1);}
 if (getValue(value3)  != "no value")
 {value3 = getValue(value3);}
if (value2 == "+"){
    value = value1.toInt()+value3.toInt();
}
 

 return value;
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
