#include <Arduino.h>

namespace Devices {
void init();
void announce_task();
void add(const char* id, const char* name, const char* ip);
void loop();
void saveToFile(const String filename);
void asString(String& res, unsigned long ttl_sec = 0);
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
    DeviceItem(const char* id, const char* name, const char* ip) {
        strlcpy(_id, id, sizeof(_id));
        strlcpy(_name, name, sizeof(_name));
        strlcpy(_ip, ip, sizeof(_ip));
        _timestamp = millis();
    }

    void update(const char* name, const char* ip) {
        strlcpy(_name, name, sizeof(_name));
        strlcpy(_ip, ip, sizeof(_ip));
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
        snprintf(buf, sizeof(buf), "%s;%s;%s;%lu", _id, _name, _ip, _timestamp);
        return buf;
    }
};

class DeviceList {
   public:
    DeviceList(){};

    DeviceItem* add(const char* id, const char* name, const char* ip) {
        _items.push_back(DeviceItem{id, name, ip});
        return last();
    }

    size_t size() {
        return _items.size();
    }

    DeviceItem* at(size_t i) {
        return &_items.at(i);
    }

    DeviceItem* get(const char* id) {
        for (size_t i = 0; i < _items.size(); i++) {
            if (!strcmp(id, _items.at(i).id())) {
                return &_items.at(i);
            }
        }
        return nullptr;
    }

   private:
    DeviceItem* last() {
        return _items.size() ? &_items.at(_items.size() - 1) : nullptr;
    }

   private:
    std::vector<DeviceItem> _items;
};
