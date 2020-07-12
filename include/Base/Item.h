#pragma once

#include "Named.h"
#include "Assigned.h"
#include "Value.h"

class Item : public Named,
             public Assigned,
             public Value {
   public:
    Item(const String& name, const String& assign) : Named{name.c_str()},
                                                     Assigned{assign.c_str()} {};
};