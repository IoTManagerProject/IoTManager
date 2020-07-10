#include "Events.h"

#include "Config.h"
#include "Utils/PrintMessage.h"

namespace Events {

StringQueue _events;

StringQueue* get() {
    return &_events;
}

void fire(String name, String param) {
    fire(name + param);
}

void fire(const String& name) {
    if (config.general()->isScenarioEnabled()) {
        _events.push(name);
    }
}
}  // namespace Events