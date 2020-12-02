#include "Class/ScenarioClass3.h"
#include "MqttClient.h"
#include "RemoteOrdersUdp.h"
Scenario* myScenario;

void eventGen2(String eventName, String eventValue) {
    if (!jsonReadBool(configSetupJson, "scen")) {
        return;
    }
    String event = eventName + " " + eventValue + ",";
    eventBuf += event;

    streamEventUDP(event);
}

void streamEventUDP(String event) {
    #ifdef UDP_ENABLED

    if (!jsonReadBool(configSetupJson, "snaUdp")) {
        return;
    }
    
    if (event.indexOf("timenow") == -1) {
        event = "iotm;event:" + event;
        asyncUdp.broadcastTo(event.c_str(), 4210);
    }
    #endif
}