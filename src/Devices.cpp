#include "Devices.h"

#include "Broadcast.h"
#include "Config.h"
#include "Utils/SysUtils.h"
#include "Utils/FileUtils.h"
#include "Utils/PrintMessage.h"

static const char* MODULE = "Devices";

namespace Devices {

DeviceList* _list;

void init() {
    _list = new DeviceList();
}

void loadFromFile() {
}

void asString(String& res, unsigned long ttl_sec) {
    for (size_t i = 0; i < _list->size(); i++) {
        DeviceItem* item = _list->at(i);
        if (ttl_sec > 0 && item->lastseen() > ttl_sec) {
            continue;
        }
        res += item->asString();
        res += "\r\n";
    }
}

void saveToFile(const String filename) {
    auto file = LittleFS.open(filename, "w");
    if (!file) {
        pm.error("save :" + filename);
        return;
    }
    file.println("id;name;url;lastseen");
    for (size_t i = 0; i < _list->size(); i++) {
        DeviceItem* item = _list->at(i);
        String data;
        data = item->id();
        data += ";";
        data += item->name();
        data += ";";
        data += "<a href=\"http://";
        data += item->ip();
        data += "\" target=\"_blank\"\">";
        data += item->ip();
        data += "</a>";
        data += ";";
        data += item->lastseen();
        file.println(data);
    }
    file.close();
}

void add(const char* id, const char* name, const char* ip) {
    DeviceItem* item = _list->get(id);
    if (item) {
        item->update(name, ip);
        pm.info("updated");
    } else {
        _list->add(id, name, ip);
        pm.info("added");
    }
}

const String asJson() {
    String res = "{\"devices\":[";
    for (size_t i = 0; i < _list->size(); i++) {
        DeviceItem* item = _list->at(i);
        res += "{\"id\":\"";
        res += item->id();
        res += "\",";
        res += "\"name\":\"";
        res += item->name();
        res += "\",";
        res += "\"url\":\"";
        res += "<a href='http://";
        res += item->ip();
        res += "' target='_blank'>";
        res += item->ip();
        res += "</a>";
        res += "\",";
        res += "\"lastseen\":";
        res += item->lastseen();
        res += i < _list->size() - 1 ? "}," : "}";
    }
    res += "]}";
    return res;
};

}  // namespace Devices