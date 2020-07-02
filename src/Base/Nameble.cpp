#include "Base/Named.h"

Named::Named(const char* name) {
    strlcpy(_name, name, sizeof(_name));
}

const char* Named::name() {
    return _name;
}
