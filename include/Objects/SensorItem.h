#pragma once

#include "Base/Named.h"
#include "Base/Assigned.h"

class SensorItem : public Named,
                   public Assigned {
   public:
    SensorItem(const String& name, const String& assign) : Named{name.c_str()},
                                                           Assigned{assign.c_str()} {};
    int sesorRead() {
        return onRead();
    }

    virtual int onRead() = 0;
};
