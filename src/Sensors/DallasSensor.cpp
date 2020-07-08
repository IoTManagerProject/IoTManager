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

DallasSensor *DallasSensors::add(String name, OneBusItem item) {
    for (size_t i = 0; i < _items.size(); i++) {
        auto item = _items.at(i);
        if (name.equals(item.name())) {
            return &_items.at(i);
        }
    }
    _items.push_back(DallasSensor(name.c_str(), item.getAddress()));
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

void loop() {
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
}  // namespace Dallas