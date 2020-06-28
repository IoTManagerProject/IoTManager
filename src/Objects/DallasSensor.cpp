#include "Objects/DallasSensors.h"

DallasSensors dallasSensors;

DallasTemperature *dallasTemperature;

DallasSensors::DallasSensors(){};

DallasSensor_t *DallasSensors::create(uint8_t address, String name) {
    for (size_t i = 0; i < _items.size(); i++) {
        auto item = _items.at(i);
        if (item.address == address) {
            item.name = name;
            return &_items.at(i);
        }
    }
    // Добавляем новый
    DallasSensor_t newItem{address, name};
    _items.push_back(newItem);
    return get(address);
}

DallasSensor_t *DallasSensors::get(String name) {
    for (size_t i = 0; i < _items.size(); i++) {
        auto item = _items.at(i);
        if (item.name == name) {
            return &_items.at(i);
        }
    }
    return nullptr;
}

DallasSensor_t *DallasSensors::get(uint8_t address) {
    for (size_t i = 0; i < _items.size(); i++) {
        auto item = _items.at(i);
        if (item.address == address) {
            return &_items.at(i);
        }
    }
    return nullptr;
}

DallasSensor_t *DallasSensors::at(size_t index) {
    return &_items.at(index);
}

size_t DallasSensors::count() {
    return _items.size();
}
