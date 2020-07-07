#include "Sensors/OneWireBus.h"

OneWireBus onewire;

OneWireBus::OneWireBus() : _pin{0}, _bus{nullptr} {};

OneWireBus::~OneWireBus() {
    delete _bus;
}

void OneWireBus::addItem(const uint8_t addr[8]) {
    for (size_t i = 0; i < _items.size(); i++) {
        OneBusItem* item = &_items.at(i);
        if (item->equals(addr)) {
            return;
        }
    }
    _items.push_back(OneBusItem{addr});
}

const String OneWireBus::asJson() {
    char buf[16];
    String res = "{\"onewire\":[";
    for (size_t i = 0; i < _items.size(); i++) {
        OneBusItem* item = &_items.at(i);
        res += "{\"num\":";
        res += itoa(i + 1, buf, DEC);
        res += ",\"addr\":\"";
        res += item->getAddress();
        res += "\"";
        res += ",\"family\":\"";
        res += item->getFamily();
        res += "\"";
        res += ",\"url\":\"";
        res += item->getAddUrl();
        res += "\"";
        res += i < _items.size() - 1 ? "}," : "}";
    }
    res += "]}";
    return res;
};

OneWire* OneWireBus::get() {
    return _bus;
}

bool OneWireBus::attached() {
    return _bus != nullptr;
}

void OneWireBus::attach(uint8_t pin) {
    if (_bus) {
        delete _bus;
    }
    _bus = new OneWire(pin);
}
