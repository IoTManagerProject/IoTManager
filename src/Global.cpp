#include "Global.h"

#include "Base/Item.h"

KeyValueStore options;
KeyValueStore liveData;
KeyValueFile runtime(DEVICE_RUNTIME_FILE);

TickerScheduler ts(SYS_MEMORY + 1);
StringCommand sCmd;

AsyncWebServer server{80};
AsyncWebSocket ws{"/ws"};
AsyncEventSource events{"/events"};

String prex = "";
String all_widgets = "";
String order_loop = "";

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

void config_init() {
    load_config();

    runtime.load();
    runtime.write("chipID", getChipId());
    runtime.write("firmware_version", FIRMWARE_VERSION);

    prex = config.mqtt()->getPrefix() + "/" + getChipId();
}

void configAdd(const String& str) {
    addFile(DEVICE_COMMAND_FILE, str);
}

void setPreset(size_t num) {
    copyFile(getConfigFile(num, CT_CONFIG), DEVICE_COMMAND_FILE);
    copyFile(getConfigFile(num, CT_SCENARIO), DEVICE_SCENARIO_FILE);
    device_init();
}

void device_init() {
    clearWidgets();
    fileExecute(DEVICE_COMMAND_FILE);
    Scenario::init();
}
