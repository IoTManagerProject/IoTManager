#include "Class/ScenarioClass3.h"
#include "MqttClient.h"
Scenario* myScenario;

void eventGen2(String eventName, String eventValue) {
    if (!jsonReadBool(configSetupJson, "scen")) {
        return;
    }
    //Serial.println(eventName + " " + eventValue);
    eventBuf += eventName + " " + eventValue + ",";

    //publish(mqttPrefix, eventName + " " + eventValue + ",");
}