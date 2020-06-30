#include "Global.h"

#ifdef WS_enable
AsyncWebSocket ws;
//AsyncEventSource events;
#endif

Clock* timeNow;
TickerScheduler ts(SYS_STAT + 1);

StringCommand sCmd;
AsyncWebServer server(80);

/*
* Global vars
*/
// Json
String configSetupJson = "{}";
String configLiveJson = "{}";
String configOptionJson = "{}";

String chipId = "";
String prex = "";
String all_widgets = "";
String order_loop = "";

// Logging
String logging_value_names_list;
int enter_to_logging_counter;

String lastVersion = "";
boolean just_load = true;

// Async actions
boolean perform_updates_check = false;
boolean perform_upgrade = false;

boolean mqttParamsChangedFlag = false;
boolean udp_data_parse = false;
boolean broadcast_mqtt_settings = false;

BusScanner_t bus_to_scan;
boolean perform_bus_scanning = false;

void saveConfig() {
    config.save(configSetupJson);
    writeFile(String("config.json"), configSetupJson);
}

void enableScenario(boolean enable) {
    config.general()->enableScenario(enable);
    if (enable) {
        Scenario::load();
    }
    saveConfig();
}

void fireEvent(String name) {
    if (config.general()->isScenarioEnabled()) {
        events.push(name);
    }
}

// событие: имя + Set + номер
void fireEvent(String name, String param) {
    fireEvent(name + param);
}

const String readLiveData(const String& obj) {
    return jsonReadStr(configLiveJson, obj);
}

const String writeLiveData(const String& obj, const String& value) {
    return jsonWriteStr(configLiveJson, obj, value);
}
