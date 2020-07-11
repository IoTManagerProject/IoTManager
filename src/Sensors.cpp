#include "Sensors.h"

#include "Events.h"
#include "Utils/PrintMessage.h"

const static char* MODULE = "Sensors";

namespace Sensors {
std::vector<SensorItem*> _items;

AnalogSensor* add(const String& name, const String& pin) {
    pm.info("name: " + name + ", pin: " + pin);
    _items.push_back(new AnalogSensor(name, pin));
    return (AnalogSensor*)last();
}

SensorItem* last() {
    return _items.at(_items.size() - 1);
}

void update() {
    for (auto item : _items) {
        MqttClient::publishStatus(VT_INT, item->getName(), String(item->read(), DEC));
        Events::fire(item);
    }
}
}