#include "Objects/ServoItems.h"
#include "Base/Value.h"
#ifdef ESP8266
#include <Servo.h>
#else
#include <ESP32Servo.h>
#endif

ServoItems myServo;

ServoItems::ServoItems(){};

BaseServo* ServoItems::add(const String& name, const String& pin, const String& value,
                           const String& in_min, const String& in_max, const String& out_min, const String& out_max) {
    BaseServo item{name, pin, value};

    item.setMap(in_min.toInt(), in_max.toInt(), out_min.toInt(), out_max.toInt());

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
