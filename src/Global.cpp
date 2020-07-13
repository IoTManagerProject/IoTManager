#include "Global.h"

#include "Widgets.h"
#include "Logger.h"
#include "MqttWriter.h"

KeyValueStore options;
KeyValueStore liveData;
KeyValueFile runtime(DEVICE_RUNTIME_FILE);

TickerScheduler ts(SYS_MEMORY + 1);

AsyncWebServer server{80};
AsyncWebSocket ws{"/ws"};
AsyncEventSource events{"/events"};

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

bool isExcludedKey(const String& key) {
    return key.equals("time") ||
           key.equals("name") ||
           key.equals("lang") ||
           key.equals("ip") ||
           key.endsWith("_in");
}

void publishState() {
    liveData.forEach([](const ValueType_t type, const String& key, const String& value) {
        if (!isExcludedKey(key)) {
            MqttClient::publishStatus(key, value, type);
        }
    });
}

void publishWidgets() {
    Writer* writer = MqttClient::getWriter("config");
    Widgets::forEach([writer](String json) {
        writer->begin(json.length());
        writer->write(json.c_str());
        writer->end();
        return true;
    });
    delete writer;
}

void publishCharts() {
    Logger::forEach([](LoggerTask* task) {
        return true;
    });
}

void config_init() {
    load_config();

    runtime.load();
    runtime.write("chipID", getChipId());
    runtime.write("firmware_version", FIRMWARE_VERSION);
    runtime.write("mqtt_prefix", config.mqtt()->getPrefix() + "/" + getChipId());
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
    Widgets::clear();
    fileExecute(DEVICE_COMMAND_FILE);
    Scenario::init();
}
