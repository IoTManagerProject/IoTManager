#include "Base/Assigned.h"

Assigned::Assigned(const char* assigned) {
    strlcpy(_assigned, assigned, sizeof(_assigned));
}

const char* Assigned::assigned() {
    return _assigned;
}

uint8_t Assigned::getPin() const {
    return String(_assigned).toInt();
}