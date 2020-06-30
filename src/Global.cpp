#include "Global.h"

#ifdef WS_enable
AsyncWebSocket ws;
//AsyncEventSource events;
#endif

Clock* timeNow;
TickerScheduler ts(SYS_STAT + 1);
WiFiClient espClient;
PubSubClient mqtt(espClient);
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
boolean checkUpdatesFlag = false;
boolean updateFlag = false;

boolean mqttParamsChanged = false;
boolean udp_data_parse = false;
boolean mqtt_send_settings_to_udp = false;

BusScanner_t busToScan;
boolean busScanFlag = false;

void saveConfig() {
    configSetup.save(configSetupJson);
    writeFile(String("config.json"), configSetupJson);
}

void enableScenario(boolean enable) {
    configSetup.enableScenario(enable);
    if (configSetup.isScenarioEnabled()) {
        Scenario::load();
    }
    saveConfig();
}

void fireEvent(String name) {
    if (configSetup.isScenarioEnabled()) {
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
