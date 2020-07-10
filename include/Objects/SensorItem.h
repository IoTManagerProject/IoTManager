#pragma once

#include "Base/Named.h"
#include "Base/Assigned.h"
#include "Base/ValueMap.h"

class SensorItem : public Named,
                   public Assigned,
                   public ValueMap {
   public:
    SensorItem(const String& name, const String& assign) : Named{name.c_str()},
                                                           Assigned{assign},
                                                           ValueMap{} {};
};
