#include "Global.h"

#include "Objects/ServoItems.h"
#include "Objects/PwmItems.h"

#ifdef WS_enable
AsyncWebSocket ws;
//AsyncEventSource events;
#endif

static const char* MODULE = "Global";

KeyValueStore options;
KeyValueStore liveData;
KeyValueFile runtime(DEVICE_RUNTIME_FILE);

Clock* timeNow;
TickerScheduler ts(SYS_STAT + 1);
StringCommand sCmd;
AsyncWebServer server(80);

/*
* Global vars
*/
// Json
String prex = "";
String all_widgets = "";
String order_loop = "";

String lastVersion = "";
boolean just_load = true;

boolean mqtt_restart_flag = false;
boolean udp_data_parse = false;

void save_config() {
    String buf;
    config.save(buf);
    writeFile(DEVICE_CONFIG_FILE, buf);
    config.setSynced();
}

void load_config() {
    String buf;
    if (readFile(DEVICE_CONFIG_FILE, buf)) {
        config.load(buf);
    }
}

const String readLiveData(const String& obj) {
    String res = "";
    if (obj.startsWith("pwm")) {
        String name = obj.substring(3);
        if (Stateble* item = myPwm.get(name)) {
            pm.info("read pwm: " + obj);
            res = String(item->getState());
        }
    } else {
        res = liveData.read(obj);
    }
    return res;
}
