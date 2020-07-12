#include "Objects/Buttons.h"

#include "Utils/PrintMessage.h"

static const char* MODULE = "Button";

Buttons myButtons;

Button* Buttons::add(const String& name, const String& assign, const String& value, const String& inverted) {
    pm.info("name: \"" + name + "\", pin: " + assign + ", state: " + value + ", inverted: " + inverted);

    Button item(name, assign, value);
    item.setInverted(inverted.toInt());
    item.setValue(value);

    _items.push_back(item);
    return last();
}

Button* Buttons::last() {
    return &_items.at(_items.size() - 1);
}

Button* Buttons::get(const String name) {
    for (size_t i = 0; i < _items.size(); i++) {
        auto item = &_items.at(i);
        if (name.equals(item->getName())) {
            return &_items.at(i);
        }
    }
    return NULL;
}

Button* Buttons::at(size_t num) {
    return &_items.at(num);
}

size_t Buttons::count() {
    return _items.size();
}
