#include <Arduino.h>

namespace Devices {
void add(const String& deviceId, const String& name, const String& ip);
void loop();
void get(String& result, unsigned long ttl_sec = 0);
void saveToFile(const String filename);
void clear();
const String asJson();
}  // namespace Devices

struct DeviceItem {
   private:
    char _id[32];
    char _name[32];
    char _ip[16];
    unsigned long _timestamp;

   public:
    DeviceItem(const String& id, const String& name, const String& ip) {
        strcpy(_id, id.c_str());
        strcpy(_name, name.c_str());
        strcpy(_ip, ip.c_str());
        Serial.print(name);
        Serial.print(ip);
        _timestamp = millis();
    }

    void update(const String& name, const String& ip) {
        Serial.print(name);
        Serial.print(ip);
        strcpy(_name, name.c_str());
        strcpy(_ip, ip.c_str());
        _timestamp = millis();
    }

    const char* id() {
        return _id;
    }
    const char* name() {
        return _name;
    }
    const char* ip() {
        return _ip;
    }

    unsigned long timestamp() {
        return _timestamp;
    }

    unsigned long lastseen() {
        unsigned long now = millis();
        return _timestamp > now ? 0 : (now - _timestamp) / 1000;
    }

    const String asString() {
        char buf[256];
        sprintf(buf, "%s;%s;%s;%lu", _id, _name, _ip, _timestamp);
        return buf;
    }
};

class DeviceList {
   public:
    DeviceList(){};

    DeviceItem* add(const String& id, const String& name, const String& ip) {
        _items.push_back(DeviceItem{id, name, ip});
        return last();
    }

    size_t size() {
        return _items.size();
    }

    DeviceItem* at(size_t i) {
        return &_items.at(i);
    }

    DeviceItem* get(const String& id) {
        for (size_t i = 0; i < _items.size(); i++) {
            if (id.equals(_items.at(i).id())) {
                return &_items.at(i);
            }
        }
        return NULL;
    }

    void asString(String& res, unsigned long ttl_sec) {
        for (auto item : _items) {
            if (ttl_sec && item.lastseen() > ttl_sec) {
                continue;
            }
            res += item.asString();
            res += "\r\n";
        }
    }

   private:
    DeviceItem*
    last() {
        return _items.size() ? &_items.at(_items.size() - 1) : NULL;
    }

   private:
    std::vector<DeviceItem> _items;
};
