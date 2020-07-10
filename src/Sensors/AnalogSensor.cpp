#include "Sensors/AnalogSensor.h"

#include "MqttClient.h"
#include "Events.h"
#include "Utils/PrintMessage.h"

static const char* MODULE = "AnalogSensor";

AnalogSensors analogSensor;

AnalogSensorItem* AnalogSensors::add(const String& name, const String& pin) {
    pm.info("name: " + name + ", pin: " + pin);
    _items.push_back(AnalogSensorItem(name, pin));
    return last();
}

void AnalogSensors::update() {
    for (auto item : _items) {
        int value = item.read();
        MqttClient::publishStatus(item.getName(), String(value));
        Events::fire(&item);
    }
}

AnalogSensorItem* AnalogSensors::last() {
    return &_items.at(_items.size() - 1);
}
