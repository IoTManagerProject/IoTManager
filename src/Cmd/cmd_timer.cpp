#include "Cmd.h"

#include "Collection/Timers.h"

static const char *MODULE = TAG_TIMER;

static const String getObjectName(const char *id) {
    return String(MODULE) + id;
}

void cmd_timerStart() {
    String name = getObjectName(sCmd.next());
    String period = sCmd.next();

    Timers::add(name, parsePeriod(period));
}

void cmd_timerStop() {
    String name = getObjectName(sCmd.next());

    Timers::erase(name);
}
