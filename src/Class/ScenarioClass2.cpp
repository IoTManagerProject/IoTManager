#include "Class/ScenarioClass2.h"
Scenario* myScenario;

void eventGen(String event_name, String number) {
    if (!jsonReadBool(configSetupJson, "scen")) {
        return;
    }
    SerialPrint("", "", event_name);
    eventBuf += event_name + number + ",";
}