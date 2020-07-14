#pragma once

#include <Arduino.h>

#include "Base/Named.h"
#include "Base/Assigned.h"

class Location : public Named, public Assigned {
   public:
    Location(const String& name, const String& order) : Named{name.c_str()}, Assigned{order.c_str()} {};
};
