#pragma once

#include "Named.h"
#include "Assigned.h"
#include "Stateble.h"

class Item : public Named,
             public Assigned,
             public Stateble {
   public:
    Item(const String& name, const String& assigned, const String& value) : Named{name.c_str()},
                                                                            Assigned{assigned.c_str()},
                                                                            Stateble{value.toInt()} {};
};