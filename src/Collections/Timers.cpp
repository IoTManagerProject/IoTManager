#include "Collection/Timers.h"

namespace Timers {
std::vector<Timer*> _items;

void update() {
    for (size_t i = 0; i < _items.size(); i++) {
        Timer* item = _items.at(i);
        if (item->tick()) {
            _items.erase(_items.begin() + i);
        }
    }
}

Timer* add(const String& name, unsigned long time) {
    for (size_t i = 0; i < _items.size(); i++) {
        Timer* item = _items.at(i);
        if (name.equalsIgnoreCase(item->name())) {
            item->setTime(time);
            return item;
        }
    }

    _items.push_back(new Timer{name.c_str(), time});

    return _items.at(_items.size() - 1);
}

void erase(const String& name) {
    for (size_t i = 0; i < _items.size(); i++) {
        Timer* item = _items.at(i);
        if (name.equalsIgnoreCase(item->name())) {
            _items.erase(_items.begin() + i);
            break;
        }
    }
}

}  // namespace Timers