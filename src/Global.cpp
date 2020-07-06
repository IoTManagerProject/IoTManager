#include "Global.h"

#include "Base/Item.h"

KeyValueStore options;
KeyValueStore liveData;
KeyValueFile runtime(DEVICE_RUNTIME_FILE);

Clock* timeNow;
TickerScheduler ts(SYS_MEMORY + 1);
StringCommand sCmd;

AsyncWebServer server{80};
AsyncWebSocket ws{"/ws"};
AsyncEventSource events{"/events"};

String prex = "";
String all_widgets = "";
String order_loop = "";

String lastVersion = "";

boolean perform_mqtt_restart_flag = false;

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
