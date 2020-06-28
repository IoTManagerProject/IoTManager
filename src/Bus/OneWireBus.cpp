#include "Bus/OneWireBus.h"

OneWireBus oneWireBus;

OneWireBus::OneWireBus() : _pin{0}, _bus{nullptr} {};

OneWireBus::~OneWireBus() {
    delete _bus;
}

OneWire* OneWireBus::get() {
    return _bus;
}

bool OneWireBus::exists() {
    return _bus != nullptr;
}

void OneWireBus::set(uint8_t pin) {
    if (_bus) {
        delete _bus;
    }
    _bus = new OneWire(pin);
}
