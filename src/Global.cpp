#include "Global.h"

#include "Collection/Widgets.h"
#include "Collection/Logger.h"
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
    liveData.forEach([](KeyValue* item) {
        if (!isExcludedKey(item->getKey())) {
            MqttClient::publishStatus(item->getKey(), item->getValue(), item->getType());
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

bool publishEntry(LogMetadata* meta, LogEntry entry) {
    String buf = "{\"status\":";
    buf += entry.asChartEntry();
    buf += "}";

    Writer* writer = MqttClient::getWriter(meta->getMqttTopic().c_str());
    writer->begin(buf.length());
    writer->write(buf.c_str());
    writer->end();
    delete writer;
    return true;
}

void publishCharts() {
    Logger::forEach([](LoggerTask* task) {
        task->readEntries([](LogMetadata* meta, LogEntry entry) {
            publishEntry(meta, entry);
            return true;
        });
        return true;
    });
}

void config_init() {
    load_config();

    runtime.reload();
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
