#include "Events.h"

#include "Config.h"
#include "Objects/EventQueue.h"

namespace Events {

void fire(String name, String param) {
    fire(name + param);
}

void fire(String name) {
    if (config.general()->isScenarioEnabled()) {
        events.push(name);
    }
}
}  // namespace Events