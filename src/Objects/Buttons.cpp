#include "Objects/Buttons.h"

#include "Utils/PrintMessage.h"

static const char* MODULE = "Button";

Buttons myButtons;

ButtonItem* Buttons::add(const String& name, const String& assign, const String& value, const String& inverted) {
    pm.info("name: \"" + name + "\", pin: " + assign + ", state: " + value + ", inverted: " + inverted);

    ButtonItem* item = new Button{name, assign, value, inverted};
    _items.push_back(item);
    return last();
}

ButtonItem* Buttons::last() {
    return _items.at(_items.size() - 1);
}

ButtonItem* Buttons::get(const String name) {
    for (size_t i = 0; i < _items.size(); i++) {
        auto item = _items.at(i);
        if (name.equals(item->getName())) {
            return _items.at(i);
        }
    }
    return nullptr;
}

ButtonItem* Buttons::at(size_t num) {
    return _items.at(num);
}

size_t Buttons::count() {
    return _items.size();
}
