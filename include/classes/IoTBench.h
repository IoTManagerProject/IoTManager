#pragma once
#include <Arduino.h>
#include "Global.h"
#include "classes/IoTItem.h"
#include <map>

struct ItemBench
{
    uint32_t sumloopTime = 0;
    uint32_t loopTime = 0;
    uint32_t loopTimeMax_p = 0;
    uint32_t loopTimeMax_glob = 0;
    uint32_t count = 0;
};



class IoTBench : public IoTItem
{
public:
    IoTBench(const String &parameters);
    ~IoTBench();

    virtual void preLoadFunction();
    virtual void postLoadFunction();
    virtual void preTaskFunction(const String &id);
    virtual void postTaskFunction(const String &id);
protected:
std::map<String, ItemBench *> banchItems;
};