#include "Sensors/DallasSensors.h"

#include "Global.h"
#include "Events.h"
#include "MqttClient.h"
#include "Utils/PrintMessage.h"

static const char *MODULE = "Dallas";

DallasSensors dallasSensors;

DallasSensors::DallasSensors(){

};

DallasSensor *DallasSensors::add(String name, String addr) {
    return add(name, OneWireAddress(addr));
}

DallasSensor *DallasSensors::add(String name, OneWireAddress addr) {
    auto item = get(name);
    if (item) {
        item->setAddr(addr);
    } else {
        _items.push_back(DallasSensor(name.c_str(), addr));
    }
    return get(name);
}

DallasSensor *DallasSensors::get(String name) {
    for (size_t i = 0; i < _items.size(); i++) {
        auto item = _items.at(i);
        if (name.equals(item.name())) {
            return &_items.at(i);
        }
    }
    return NULL;
}

DallasSensor *DallasSensors::at(size_t index) {
    return &_items.at(index);
}

size_t DallasSensors::count() {
    return _items.size();
}

void DallasSensors::loop() {
    for (auto item : _items) {
        if (item.update()) {
            float temp = item.getTempC();
            pm.info(item.addr()->asString() + ":" + String(temp, 2));
        }
        // liveData.writeFloat(item.name(), temp);
        // Scenario::fire(item.name());
        // MqttClient::publishStatus(item.name(), String(temp, 2));
    }
}
