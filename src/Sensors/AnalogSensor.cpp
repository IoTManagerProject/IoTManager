#include "Sensors/AnalogSensor.h"

#include "MqttClient.h"
#include "Events.h"
#include "Objects/AnalogSensorItem.h"

#include "Utils/PrintMessage.h"

static const char* MODULE = "AnalogSensor";

namespace AnalogSensor {

std::vector<AnalogSensorItem> _items;

void loop() {
    for (size_t i = 0; i < _items.size(); i++) {
        AnalogSensorItem* item = &_items.at(i);
        int value = item->read();
        MqttClient::publishStatus(item->getName(), String(value));
        Events::fire(item->getName());
    }
}

void add(const String& name, const String& pin, const String& min_value, const String& max_value, const String& min_deg, const String& max_deg) {
    _items.push_back(AnalogSensorItem(name, pin, new ValueMapper(min_value.toInt(), max_value.toInt(), min_deg.toInt(), max_deg.toInt())));
    pm.info("name: " + name + ", pin: " + pin);
}

}  // namespace AnalogSensor