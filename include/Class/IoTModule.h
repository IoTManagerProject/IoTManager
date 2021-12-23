#pragma once

#include <WString.h>

struct ModuleInfo
{
    String name;
    String key;
    String parameters;
    String type;
};

class IoTModule {
   public:
    IoTModule();
    ~IoTModule();

    virtual void* initInstance(String parameters);
    virtual ModuleInfo getInfo();
};