#pragma once

#include <Arduino.h>

#include "Base/Named.h"
#include "Base/StringQueue.h"

enum EquationSign {
    OP_EQUAL,
    OP_NOT_EQUAL,
    OP_LESS,
    OP_LESS_OR_EQAL,
    OP_GREATER,
    OP_GREATER_OR_EQAL
};

class ParamItem {
   public:
    virtual const String value();
};

class LiveParam : public ParamItem {
   public:
    LiveParam(const String& name);
    const String value() override;

   private:
    String _name;
};

class StaticParam : public ParamItem {
   public:
    StaticParam(const String& value);
    const String value() override;

   private:
    String _value;
};

class ScenarioItem {
   public:
    ScenarioItem(const String& str);
    bool isValid();
    bool isEnabled();
    bool enable(bool value);
    bool run(const String& value);

   private:
    bool equation(const String& object, const String& value);
    bool parseCondition(const String& str);
    bool parseSign(const String& str, EquationSign& sign);

   private:
    bool _valid;
    bool _enabled;
    String _commands;
    String _obj;
    EquationSign _sign;
    ParamItem* _param;
};

namespace Scenario {

void fire(Named* obj);

void fire(const String& name);

void reinit();

void init();

void loop();

void enableBlock(size_t num, boolean enable);

bool isBlockEnabled(size_t num);

}  // namespace Scenario