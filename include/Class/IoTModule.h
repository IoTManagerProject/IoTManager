#pragma once

#include <WString.h>

struct ModuleInfo
{
    String name;  //имя модуля
    String title;  //заголовок для описания модуля
    String parameters;  //параметры, которые может принять модуль и сущность
    String type;  //тип для определения сущности, которую генерирует модуль Sensor или Variable
};

class IoTModule {
   public:
    IoTModule();
    ~IoTModule();

    virtual void* initInstance(String parameters);
    virtual ModuleInfo getInfo();
    virtual void clear();
};