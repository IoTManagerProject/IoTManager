#include <Servo.h>

#include "Objects/ServoItems.h"

#include "Base/Value.h"

ServoItems myServo;

ServoItems::ServoItems(){};

BaseServo* ServoItems::add(const String& name, const String& pin, const String& value,
                           const String& min_value, const String& max_value, const String& min_deg, const String& max_deg) {
    _items.push_back(BaseServo{name, pin, value,
                               new ValueMapper(min_value.toInt(),
                                               max_value.toInt(),
                                               min_deg.toInt(),
                                               max_deg.toInt())});
    return last();
}

BaseServo* ServoItems::last() {
    return &_items.at(_items.size() - 1);
}

BaseServo* ServoItems::get(const String& name) {
    for (size_t i = 0; i < _items.size(); i++) {
        auto* item = &_items.at(i);
        if (name.equals(name)) {
            return item;
        }
    }
    return nullptr;
}

size_t ServoItems::count() {
    return _items.size();
}
