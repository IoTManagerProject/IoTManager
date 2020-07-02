#pragma once

#include <Arduino.h>

#include "Base/Item.h"

#include "Base/ValueMapper.h"

class BaseServo : public Item {
   public:
    BaseServo(const String& name, const String& pin, const String& value, ValueMapper* mapper) : Item{name, pin, value},
                                                                                                 _mapper{mapper} {
        _obj = new Servo();
    };

    void onInit() override {
        _obj->attach(this->getPin());
        updateState();
    }

    void onStateChange() override {
        int value = _mapper->transform(getState());
        _obj->write(value);
    }

   private:
    Servo* _obj;
    ValueMapper* _mapper;
};
