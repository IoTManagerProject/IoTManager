#include "Global.h"

#include "LiveData.h"
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
String runtimeJson = "{}";

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

void load_runtime() {
    if (readFile(DEVICE_RUNTIME_FILE, runtimeJson)) {
        runtimeJson.replace(" ", "");
        runtimeJson.replace("\r\n", "");
    } else {
        runtimeJson = "{}";
    }
    Serial.println(runtimeJson);

    jsonWriteStr(runtimeJson, "chipID", getChipId());
    jsonWriteStr(runtimeJson, "firmware_version", FIRMWARE_VERSION);

    Serial.println(runtimeJson);
}

void save_runtime() {
    writeFile(DEVICE_RUNTIME_FILE, runtimeJson);
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
        res = LiveData::read(obj);
    }
    return res;
}
