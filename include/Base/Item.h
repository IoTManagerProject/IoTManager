#pragma once

#include "Named.h"
#include "Assigned.h"
#include "Stateble.h"

class Item : public Named,
             public Stateble {
   public:
    Item(const String& name, const String& assigned, const String& value) : Named{name.c_str()},
                                                                            Stateble{value.toInt()} {};
};