#pragma once

#include "Base/Named.h"
#include "Base/Assigned.h"
#include "Base/Readable.h"

class Sensor : public Named, public Assigned, public Readable {
   public:
    Sensor(const String& name, const String& assign) : Named{name.c_str()},
                                                       Assigned{assign.c_str()},
                                                       Readable{} {}
};