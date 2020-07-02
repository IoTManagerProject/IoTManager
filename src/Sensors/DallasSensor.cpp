#include "Sensors/DallasSensor.h"

#include "Global.h"
#include "Events.h"
#include "MqttClient.h"
#include "Utils/PrintMessage.h"

namespace Dallas {

DallasSensors dallasSensors;

DallasTemperature *dallasTemperature;
String dallas_value_name = "";
int enter_to_dallas_counter = 0;

DallasSensors::DallasSensors(){};

DallasSensor_t *DallasSensors::create(uint8_t *address, String name) {
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

DallasSensor_t *DallasSensors::get(uint8_t *address) {
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

void dallas_reading() {
    if (!dallasTemperature) {
        return;
    }

    float temp = 0;
    byte num = dallasTemperature->getDS18Count();
    String dallas_value_name_tmp_buf = dallas_value_name;
    dallasTemperature->requestTemperatures();
    for (byte i = 0; i < num; i++) {
        temp = dallasTemperature->getTempCByIndex(i);
        String buf = selectToMarker(dallas_value_name_tmp_buf, ";");
        dallas_value_name_tmp_buf = deleteBeforeDelimiter(dallas_value_name_tmp_buf, ";");
        liveData.write(buf, String(temp));
        Events::fire(buf);
        MqttClient::publishStatus(buf, String(temp));
        Serial.println("[I] sensor '" + buf + "' send date " + String(temp));
    }
}
}