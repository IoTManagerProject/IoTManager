#include "Sensors.h"

#include "Utils/PrintMessage.h"

const static char* MODULE = "Sensors";

namespace Sensors {

std::vector<BaseSensor*> _items;

BaseSensor* last() {
    return _items.at(_items.size() - 1);
}

AnalogSensor* add(const String& name, const String& pin) {
    _items.push_back(new AnalogSensor(name, pin));
    return (AnalogSensor*)last();
}

void update() {
    for (BaseSensor* item : _items) {
    
        String name = item->getName();
        String value = item->getValue();
        
        Scenario::fire(item);
        liveData.write(name, value);
        MqttClient::publishStatus(VT_INT, name, value);
        
        //MqttClient::publishStatus(VT_FLOAT, getName(), String(value, 2));
    }
}
}  // namespace Sensors