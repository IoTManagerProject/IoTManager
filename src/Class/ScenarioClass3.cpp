#include "Class/ScenarioClass3.h"
Scenario* myScenario;

//void eventGen(String event_name, String number) {
//    if (!jsonReadBool(configSetupJson, "scen")) {
//        return;
//    }
//    SerialPrint("", "", event_name);
//    eventBuf += event_name + number + ",";
//}

void eventGen2(String eventName, String eventValue) {
    if (!jsonReadBool(configSetupJson, "scen")) {
        return;
    }
    //Serial.println(eventName + " " + eventValue);
    eventBuf += eventName + " " + eventValue + ",";
}