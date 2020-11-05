#include "Class/ScenarioClass.h"
Scenario* myScenario;

void eventGen(String event_name, String number) {
    if (!jsonReadBool(configSetupJson, "scen")) {
        return;
    }
    eventBuf += event_name + number + ",";
}