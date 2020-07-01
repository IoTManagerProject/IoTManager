#include "Global.h"

#include "Objects/Pwms.h"

#ifdef WS_enable
AsyncWebSocket ws;
//AsyncEventSource events;
#endif

static const char* MODULE = "Global";

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

boolean mqttParamsChangedFlag = false;
boolean udp_data_parse = false;

void saveConfig() {
    config.save(configSetupJson);
    writeFile(String("config.json"), configSetupJson);
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
    String res = "";
    if (obj.startsWith("pwm")) {
        String name = obj.substring(3);
        if (Stateble* item = myPwms.get(name)) {
            pm.info("read pwm: " + obj);
            res = String(item->getState());
        }
    } else {
        pm.info("read live: " + obj);
        res = jsonReadStr(configLiveJson, obj);
    }
    return res;
}

const String writeLiveData(const String& obj, const String& value) {
    pm.info("write live: " + obj + " " + value);
    return jsonWriteStr(configLiveJson, obj, value);
}
